#include "pch.h"
#include <mmdeviceapi.h>
#include <atlcomcli.h>
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"

namespace rtcv {

void WaveOutHandler::clearCallbacks()
{
    onUpdate = {};
}

void WaveOutHandler::onWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret)
{
    super::onWaveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen, ret);
    if (FAILED(ret))
        return;

    auto& rec = m_records[*phwo];
    rec.is_opened = true;

    rec.data.frequency = pwfx->nSamplesPerSec;
    rec.data.channels = pwfx->nChannels;
    switch (pwfx->wBitsPerSample) {
    case 8: rec.data.format = rt::AudioFormat::U8; break;
    case 16: rec.data.format = rt::AudioFormat::S16; break;
    case 24: rec.data.format = rt::AudioFormat::S24; break;
    case 32: rec.data.format = rt::AudioFormat::S32; break;
    }
}

void WaveOutHandler::onWaveOutClose(HWAVEOUT& hwo, MMRESULT& ret)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing) {
        rec.is_playing = false;
    }
    rec.is_opened = false;

    super::onWaveOutClose(hwo, ret);
}

void WaveOutHandler::onWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end()) {
        Record tmp;
        // not sure this is always correct...
        tmp.data.frequency = 48000;
        tmp.data.channels = 2;
        tmp.data.format = rt::AudioFormat::S16;
        m_records[hwo] = tmp;
        it = m_records.find(hwo);
    }

    auto& rec = it->second;
    rec.data.data.assign(pwh->lpData, pwh->lpData + pwh->dwBufferLength);
    if (!rec.is_playing) {
        rec.is_playing = true;
    }
    if (onUpdate)
        onUpdate(rec.data);
    if (mute)
        memset(pwh->lpData, 0, pwh->dwBufferLength);

    super::onWaveOutWrite(hwo, pwh, cbwh, ret);
}

void WaveOutHandler::onWaveOutReset(HWAVEOUT& hwo, MMRESULT& ret)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing)
        rec.is_playing = false;

    super::onWaveOutReset(hwo, ret);
}


void WindowMessageHandler::onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    super::onGetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret);

    if (!rt::IsInMainThread())
        return;

    auto& server = TalkServer::getInstance();
    server.start();
    // note: server.processMessages() is not needed on CeVIO
}

} // namespace rtcv
