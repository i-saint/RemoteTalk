#include "pch.h"
#include "rtvrDSound.h"

void rtvrDSoundHandler::clearCallbacks()
{
    onPlay = {};
    onStop = {};
    onUpdateBuffer = {};
}

void rtvrDSoundHandler::afterIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret)
{
    if (ret != S_OK)
        return;

    m_buffer.frequency = pcDSBufferDesc->lpwfxFormat->nSamplesPerSec;
    m_buffer.channels = pcDSBufferDesc->lpwfxFormat->nChannels;
    switch (pcDSBufferDesc->lpwfxFormat->wBitsPerSample) {
    case 8: m_buffer.format = rt::AudioFormat::U8; break;
    case 16: m_buffer.format = rt::AudioFormat::S16; break;
    case 24: m_buffer.format = rt::AudioFormat::S24; break;
    case 32: m_buffer.format = rt::AudioFormat::S32; break;
    }
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwOffset, DWORD& dwBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret)
{
    m_lbuf1 = ppvAudioPtr1 ? *ppvAudioPtr1 : nullptr;
    m_lsize1 = pdwAudioBytes1 ? *pdwAudioBytes1 : 0;
    m_lbuf2 = ppvAudioPtr2 ? *ppvAudioPtr2 : nullptr;
    m_lsize2 = pdwAudioBytes2 ? *pdwAudioBytes2 : 0;

    m_buffer.data.reserve_discard(m_lsize1 + m_lsize2);
    m_buffer.data.resize_discard(dwBytes);
    if (ppvAudioPtr1)
        *ppvAudioPtr1 = m_buffer.data.data();
    if (ppvAudioPtr2)
        *ppvAudioPtr2 = m_buffer.data.data() + m_lsize1;
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
            memcpy(m_lbuf1, m_buffer.data.data(), m_lsize1);
        if (m_lbuf2)
            memcpy(m_lbuf2, m_buffer.data.data() + m_lsize1, m_lsize2);
    }

    pvAudioPtr1 = m_lbuf1;
    pvAudioPtr2 = m_lbuf2;

    if (onUpdateBuffer)
        onUpdateBuffer(m_buffer);
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret)
{
    if (onPlay)
        onPlay();
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret)
{
    if (onStop)
        onStop();
}

void rtvrDSoundHandler::afterIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition, HRESULT& ret)
{
}
