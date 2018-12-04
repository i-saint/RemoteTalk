#include "pch.h"
#include "rtvrDSound.h"


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
}

void rtvrDSoundHandler::beforeIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2)
{
    m_buffer.data.resize_discard(dwAudioBytes1 + dwAudioBytes2);
    if (pvAudioPtr1)
        memcpy(m_buffer.data.data(), pvAudioPtr1, dwAudioBytes1);
    if (pvAudioPtr2)
        memcpy(m_buffer.data.data() + dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);

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
