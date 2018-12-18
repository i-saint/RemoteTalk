#include "pch.h"
#include <mmdeviceapi.h>
#include <atlcomcli.h>
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"


void rtcvWindowMessageHandler::afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    auto& server = rtcvTalkServer::getInstance();
    server.start();
}


void rtcvWaveOutHandler::clearCallbacks()
{
    onUpdate = {};
}

void rtcvWaveOutHandler::afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret)
{
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

void rtcvWaveOutHandler::beforeWaveOutClose(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing) {
        rec.is_playing = false;
    }
    rec.is_opened = false;
}

static bool GetSystemSoundSettings(rt::AudioData& dst)
{
    HRESULT hr;
    CComPtr<IMMDevice> pDevice;
    CComPtr<IMMDeviceEnumerator> pEnumerator;
    CComPtr<IPropertyStore> store;

    hr = CoInitialize(nullptr);
    if (FAILED(hr))
        return false;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)&pEnumerator);
    if (FAILED(hr))
        return false;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
    if (FAILED(hr))
        return false;

    hr = pDevice->OpenPropertyStore(STGM_READ, &store);
    if (FAILED(hr))
        return false;

    const PROPERTYKEY _PKEY_AudioEngine_DeviceFormat = { 0xf19f064d, 0x82c, 0x4e27, 0xbc, 0x73, 0x68, 0x82, 0xa1, 0xbb, 0x8e, 0x4c, 0 };
    PROPVARIANT prop;
    hr = store->GetValue(_PKEY_AudioEngine_DeviceFormat, &prop);
    if (FAILED(hr))
        return false;

    auto wf = (PWAVEFORMATEX)prop.blob.pBlobData;
    dst.frequency = wf->nSamplesPerSec;
    dst.channels = std::min((int)wf->nChannels, 2); // ???
    switch (wf->wBitsPerSample) {
    case 8: dst.format = rt::AudioFormat::U8; break;
    case 16: dst.format = rt::AudioFormat::S16; break;
    case 24: dst.format = rt::AudioFormat::S24; break;
    case 32: dst.format = rt::AudioFormat::S32; break;
    }
    return true;
}

void rtcvWaveOutHandler::beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end()) {
        Record tmp;
        if (GetSystemSoundSettings(tmp.data)) {
            m_records[hwo] = tmp;
            it = m_records.find(hwo);
        }
        else
            return;
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
}

void rtcvWaveOutHandler::beforeWaveOutReset(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing)
        rec.is_playing = false;
}
