#pragma once
#include "RemoteTalk/RemoteTalk.h"

class rtvrDSoundHandler : public rt::DSoundHandlerBase
{
public:
    rtDefSingleton(rtvrDSoundHandler);

    bool mute = false;
    std::function<void()> onPlay, onStop;
    std::function<void(const rt::AudioData&)> onUpdate;

    void clearCallbacks();
    void update(IDirectSoundBuffer *_this);

protected:
    void afterIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret) override;
    void afterIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwWriteCursor, DWORD& dwWriteBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret) override;
    void afterIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret) override;
    void beforeIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition) override;
    void afterIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret) override;
    void beforeIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2) override;

private:
    rt::RawVector<char> m_buffer;
    rt::AudioData m_data;

    void *m_lbuf1, *m_lbuf2;
    uint32_t m_lsize1, m_lsize2;
    uint32_t m_offset = 0;
    uint32_t m_position = 0;
    bool m_playing = false;
};
