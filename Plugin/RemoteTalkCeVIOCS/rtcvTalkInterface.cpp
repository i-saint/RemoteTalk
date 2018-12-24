#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvTalkInterfaceManaged.h"
#include "rtcvTalkInterface.h"

namespace rtcv {

TalkInterface::TalkInterface()
{
}

TalkInterface::~TalkInterface()
{
    auto mod = ::GetModuleHandleA(rtcvHookDll);
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void TalkInterface::release() { /*do nothing*/ }
const char* TalkInterface::getClientName() const { return rtcvHostName; }
int TalkInterface::getPluginVersion() const { return rtPluginVersion; }
int TalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool TalkInterface::getParams(rt::TalkParams& params) const
{
    return TalkInterfaceManaged::getInstance()->getParams(params);
}

bool TalkInterface::setParams(const rt::TalkParams& params)
{
    m_params = params;
    return TalkInterfaceManaged::getInstance()->setParams(params);
}

int TalkInterface::getNumCasts() const
{
    if (m_casts.empty())
        m_casts = TalkInterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

rt::CastInfo* TalkInterface::getCastInfo(int i) const
{
    if (i >= 0 && i < (int)m_casts.size())
        return &m_casts[i];
    return nullptr;
}

bool TalkInterface::setText(const char *text)
{
    TalkInterfaceManaged::getInstance()->setText(text);
    return true;
}

void TalkInterface::setTempFilePath(const char *path)
{
    TalkInterfaceManaged::getInstance()->setTempFilePath(path);
}



bool TalkInterface::isReady() const
{
    return !TalkInterfaceManaged::getInstance()->isPlaying();
}

bool TalkInterface::isPlaying() const
{
    return TalkInterfaceManaged::getInstance()->isPlaying();
}

bool TalkInterface::play()
{
    m_is_playing = true;
    if (TalkInterfaceManaged::getInstance()->talk()) {
        return true;
    }
    else {
        m_is_playing = false;
        return false;
    }
}

bool TalkInterface::stop()
{
    if (TalkInterfaceManaged::getInstance()->stop()) {
        m_is_playing = false;
        return true;
    }
    else {
        return false;
    }
}

bool TalkInterface::wait()
{
    if (TalkInterfaceManaged::getInstance()->wait()) {
        m_is_playing = false;
        return true;
    }
    else {
        return false;
    }
}

#ifdef rtDebug
bool TalkInterface::onDebug()
{
    return true;
}
#endif

} // namespace rtcv
