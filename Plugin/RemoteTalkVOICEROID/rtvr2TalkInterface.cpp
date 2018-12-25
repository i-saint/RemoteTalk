#include "pch.h"
#include "rtvr2Common.h"
#include "rtvr2TalkInterfaceManaged.h"
#include "rtvr2TalkInterface.h"

namespace rtvr2 {

TalkInterface::TalkInterface()
{
}

TalkInterface::~TalkInterface()
{
    auto mod = ::GetModuleHandleA(rtvr2HookDll);
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void TalkInterface::release() { /*do nothing*/ }
const char* TalkInterface::getClientName() const { return rtvr2HostName; }
int TalkInterface::getPluginVersion() const { return rtPluginVersion; }
int TalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool TalkInterface::getParams(rt::TalkParams& params) const
{
    return TalkInterfaceManaged::getInstance()->getParams(params);
}

bool TalkInterface::setParams(const rt::TalkParams& params)
{
    return TalkInterfaceManaged::getInstance()->setParams(params);
}

int TalkInterface::getNumCasts() const
{
    m_casts = TalkInterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

const rt::CastInfo* TalkInterface::getCastInfo(int i) const
{
    if (i >= 0 && i < (int)m_casts.size())
        return &m_casts[i];
    return nullptr;
}

bool TalkInterface::setText(const char *text)
{
    return TalkInterfaceManaged::getInstance()->setText(text);
}

bool TalkInterface::isReady() const
{
    return !m_is_playing;
}

bool TalkInterface::isPlaying() const
{
    return m_is_playing;
}

bool TalkInterface::play()
{
    if (m_is_playing)
        return false;

    if (TalkInterfaceManaged::getInstance()->talk()) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool TalkInterface::stop()
{
    if (!m_is_playing)
        return false;
    return TalkInterfaceManaged::getInstance()->stop();
}


bool TalkInterface::setCast(int v)
{
    return TalkInterfaceManaged::getInstance()->setCast(v);
}

bool TalkInterface::isMainWindowVisible()
{
    return TalkInterfaceManaged::getInstance()->isMainWindowVisible();
}

bool TalkInterface::prepareUI()
{
    return TalkInterfaceManaged::getInstance()->prepareUI();
}

void TalkInterface::onPlay()
{
    m_is_playing = true;
}

void TalkInterface::onStop()
{
    m_is_playing = false;
}


#ifdef rtDebug
static void PrintControlInfo(System::Windows::DependencyObject^ obj, int depth = 0)
{
    std::string t;
    for (int i = 0; i < depth; ++i)
        t += "  ";
    t += ToStdString(obj->GetType()->FullName);
    t += "\n";
    ::OutputDebugStringA(t.c_str());

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        PrintControlInfo(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), depth + 1);
}

bool TalkInterface::onDebug()
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            PrintControlInfo(w);
        }
    }
    return true;
}
#endif

} // namespace rtvr2
