#include "pch.h"
#include "rtspHookHandler.h"

namespace rtsp {

void WaveOutHandler::clearCallbacks()
{
    onUpdate = {};
}

void WaveOutHandler::onWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret)
{
    super::onWaveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen, ret);
    if (FAILED(ret))
        return;

    auto& rec = m_records[phwo ? *phwo : nullptr];
    rec.is_opened = true;
    rec.wave_format = *pwfx;

    rec.data.frequency = rec.wave_format.nSamplesPerSec;
    rec.data.channels = rec.wave_format.nChannels;
    switch (rec.wave_format.wBitsPerSample) {
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
    if (it == m_records.end())
        return;

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
    mute = false;

    super::onWaveOutReset(hwo, ret);
}

} // namespace rtsp
