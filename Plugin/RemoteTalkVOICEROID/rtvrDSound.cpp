#include "pch.h"
#include "rtvrDSound.h"

void rtvrDSoundHandler::clearCallbacks()
{
    onPlay = {};
    onStop = {};
    onUpdate = {};
}

void rtvrDSoundHandler::update(IDirectSoundBuffer *_this)
{
    DWORD cur, wcur;
    _this->GetCurrentPosition(&cur, &wcur);
    if (cur == m_position)
        return;

    if (cur > m_position) {
        m_data.data.assign(&m_buffer[m_position], &m_buffer[cur]);
    }
    else {
        m_data.data.assign(&m_buffer[m_position], m_buffer.end());
        m_data.data.insert(m_data.data.end(), m_buffer.begin(), &m_buffer[cur]);
    }
    m_position = cur;
    if (m_playing && onUpdate)
        onUpdate(m_data);
}

void rtvrDSoundHandler::afterIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret)
{
    if (ret != S_OK)
        return;

    m_buffer.resize_zeroclear(pcDSBufferDesc->dwBufferBytes);
    m_data.data.reserve(pcDSBufferDesc->dwBufferBytes);

    m_data.frequency = pcDSBufferDesc->lpwfxFormat->nSamplesPerSec;
    m_data.channels = pcDSBufferDesc->lpwfxFormat->nChannels;
    switch (pcDSBufferDesc->lpwfxFormat->wBitsPerSample) {
    case 8: m_data.format = rt::AudioFormat::U8; break;
    case 16: m_data.format = rt::AudioFormat::S16; break;
    case 24: m_data.format = rt::AudioFormat::S24; break;
    case 32: m_data.format = rt::AudioFormat::S32; break;
    }
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwWriteCursor, DWORD& dwWriteBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret)
{
    update(_this);

    m_lbuf1 = ppvAudioPtr1 ? *ppvAudioPtr1 : nullptr;
    m_lsize1 = pdwAudioBytes1 ? *pdwAudioBytes1 : 0;
    m_lbuf2 = ppvAudioPtr2 ? *ppvAudioPtr2 : nullptr;
    m_lsize2 = pdwAudioBytes2 ? *pdwAudioBytes2 : 0;

    m_offset = dwWriteCursor;
    if (ppvAudioPtr1)
        *ppvAudioPtr1 = &m_buffer[m_offset];
    if (ppvAudioPtr2)
        *ppvAudioPtr2 = &m_buffer[0];
}

void rtvrDSoundHandler::beforeIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2)
{
    if (mute) {
        if (m_lbuf1)
            memset(m_lbuf1, 0, m_lsize1);
        if (m_lbuf2)
            memset(m_lbuf2, 0, m_lsize2);
    }
    else {
        if (m_lbuf1)
            memcpy(m_lbuf1, &m_buffer[m_offset], m_lsize1);
        if (m_lbuf2)
            memcpy(m_lbuf2, &m_buffer[0], m_lsize2);
    }

    pvAudioPtr1 = m_lbuf1;
    pvAudioPtr2 = m_lbuf2;
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret)
{
    m_playing = true;
    if (onPlay)
        onPlay();
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret)
{
    update(_this);
    m_playing = false;
    if (onStop)
        onStop();
}

void rtvrDSoundHandler::beforeIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition)
{
    update(_this);
    m_position = dwNewPosition;
}
