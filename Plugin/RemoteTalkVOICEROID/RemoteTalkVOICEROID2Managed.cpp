#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalkVOICEROID2Controller.h"

using namespace System;
using namespace System::Windows;
using namespace System::Windows::Forms;
using namespace System::Windows::Interop;
using namespace System::Runtime::InteropServices;


ref class rtvr2Context
{
public:
    static rtvr2Context^ getInstance();
    Control^ getTextArea();

private:
    static rtvr2Context s_instance;

    Control^ m_ctrl_text;
    Control^ m_ctrl_play;
    Control^ m_ctrl_save;
};

class rtvr2Controller : public rtvr2IController
{
public:
    ~rtvr2Controller() override;
    void setTalker(int id) override;
    void setVoiceParams(const Params& v) override;
    void setStyleParams(const Style& v) override;
    void talk(const std::string& text) override;

    void dbgListWindows(std::vector<std::string>& dst) override;
};


static HWND FindWindowByTypeName(String^ name)
{
    std::vector<HWND> windows;
    rt::EnumerateAllWindows([&windows](HWND hw) {
        windows.push_back(hw);
    });

    for (auto hw : windows) {
        auto ctrl = Control::FromHandle(IntPtr(hw));
        if (ctrl != nullptr) {
            auto name = ctrl->GetType()->FullName;
            if (name == name) {
                return hw;
            }
        }
    }
    return nullptr;
}

static std::vector<HWND> GetChildWindows(HWND parent)
{
    std::vector<HWND> ret;
    rt::EnumerateChildWindowsRecirsive(parent, [&ret](HWND hw) {
        ret.push_back(hw);
    });
    return ret;
}

static std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return (const char*)ptr.ToPointer();
}


rtvr2Context^ rtvr2Context::getInstance()
{
    return %s_instance;
}

Control^ rtvr2Context::getTextArea()
{
    if (!m_ctrl_text) {
        if (auto tev = FindWindowByTypeName("AI.Talk.Editor.TextEditView")) {
            auto children = GetChildWindows(tev);
            if (children.size() > 24) {
                m_ctrl_text = Control::FromHandle(IntPtr(children[4]));
                m_ctrl_play = Control::FromHandle(IntPtr(children[6]));
                m_ctrl_save = Control::FromHandle(IntPtr(children[24]));
            }
        }
    }
    return m_ctrl_text;
}



rtvr2IController::~rtvr2IController()
{
}

rtvr2Controller::~rtvr2Controller()
{
}

void rtvr2Controller::setTalker(int id)
{
}

void rtvr2Controller::setVoiceParams(const Params & v)
{
}

void rtvr2Controller::setStyleParams(const Style & v)
{
}

void rtvr2Controller::talk(const std::string & text)
{
}

static void GetControlInfo(System::Windows::DependencyObject^ obj, std::vector<std::string>& dst)
{
    dst.push_back(ToStdString(obj->GetType()->FullName));

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        GetControlInfo(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), dst);
}

void rtvr2Controller::dbgListWindows(std::vector<std::string>& dst)
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            GetControlInfo(w, dst);
        }
    }
}

rtExport rtvr2IController* rtvr2GetController()
{
    static rtvr2Controller s_instance;
    return &s_instance;
}
