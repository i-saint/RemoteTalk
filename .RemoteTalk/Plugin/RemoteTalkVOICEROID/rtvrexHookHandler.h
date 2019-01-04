#pragma once
#include "RemoteTalk/RemoteTalk.h"

namespace rtvrex {

class DSoundHandler : public rt::DSoundHandlerBase
{
using super = rt::DSoundHandlerBase;
public:
    rtDefSingleton(DSoundHandler);

    bool mute = false;
    int margin = 88200 / 10; // 1/10 sec
 
    void update(IDirectSoundBuffer *_this, bool apply_margin = false);

protected:
    void onIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwWriteCursor, DWORD& dwWriteBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret) override;
    void onIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret) override;
    void onIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition, HRESULT& ret) override;
    void onIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret) override;
    void onIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2, HRESULT& ret) override;

private:
    rt::RawVector<char> m_buffer;
    rt::AudioData m_data;

    void *m_lbuf1, *m_lbuf2;
    uint32_t m_lsize1, m_lsize2;
    uint32_t m_offset = 0;
    uint32_t m_position = 0;
    bool m_playing = false;
};

class WindowMessageHandler : public rt::WindowMessageHandlerBase
{
using super = rt::WindowMessageHandlerBase;
public:
    rtDefSingleton(WindowMessageHandler);
    void onGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
    void onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;

    void update(const MSG& msg);

    const int interval = 33;
    int frame = 0;
    UINT_PTR timer_id = 0;
};


} // namespace rtvrex
