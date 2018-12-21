#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvInterfaceManaged.h"
#include "rtcvInterface.h"


rtcvTalkInterface::rtcvTalkInterface()
{
}

rtcvTalkInterface::~rtcvTalkInterface()
{
    auto mod = ::GetModuleHandleA(rtcvHookDll);
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtcvTalkInterface::release() { /*do nothing*/ }
const char* rtcvTalkInterface::getClientName() const { return rtcvHostName; }
int rtcvTalkInterface::getPluginVersion() const { return rtPluginVersion; }
int rtcvTalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool rtcvTalkInterface::getParams(rt::TalkParams& params) const
{
    return rtcvInterfaceManaged::getInstance()->getParams(params);
}

bool rtcvTalkInterface::setParams(const rt::TalkParams& params)
{
    m_params = params;
    return rtcvInterfaceManaged::getInstance()->setParams(params);
}

int rtcvTalkInterface::getNumCasts() const
{
    if (m_casts.empty())
        m_casts = rtcvInterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

rt::CastInfo* rtcvTalkInterface::getCastInfo(int i) const
{
    if (i >= 0 && i < (int)m_casts.size())
        return &m_casts[i];
    return nullptr;
}

bool rtcvTalkInterface::setText(const char *text)
{
    return rtcvInterfaceManaged::getInstance()->setText(text);
}

bool rtcvTalkInterface::isReady() const
{
    return !rtcvInterfaceManaged::getInstance()->isPlaying();
}

bool rtcvTalkInterface::isPlaying() const
{
    return rtcvInterfaceManaged::getInstance()->isPlaying();
}

bool rtcvTalkInterface::play()
{
    m_is_playing = true;
    if (rtcvInterfaceManaged::getInstance()->talk()) {
        return true;
    }
    else {
        m_is_playing = false;
        return false;
    }
}

bool rtcvTalkInterface::stop()
{
    if (rtcvInterfaceManaged::getInstance()->stop()) {
        m_is_playing = false;
        return true;
    }
    else {
        return false;
    }
}

bool rtcvTalkInterface::wait()
{
    if (rtcvInterfaceManaged::getInstance()->wait()) {
        m_is_playing = false;
        return true;
    }
    else {
        return false;
    }
}

#ifdef rtDebug
bool rtcvTalkInterface::onDebug()
{
    return true;
}
#endif
