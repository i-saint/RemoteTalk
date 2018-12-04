#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalkVOICEROID2Controller.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


ref class rtvr2Context
{
public:
    static rtvr2Context^ getInstance();

    void setTalker(int id);
    void setVoiceParams(const rtvr2Params& v);
    void setStyleParams(const rtvr2Style& v);
    bool talk(const std::string& text);

private:
    static rtvr2Context s_instance;

    bool setupControls();

    System::Windows::Controls::TextBox^ m_tb_text;
    System::Windows::Controls::Button^ m_bu_play;
    System::Windows::Controls::Button^ m_bu_save;
};

class rtvr2Controller : public rtvr2IController
{
public:
    ~rtvr2Controller() override;
    void setTalker(int id) override;
    void setVoiceParams(const rtvr2Params& v) override;
    void setStyleParams(const rtvr2Style& v) override;
    bool talk(const std::string& text) override;

    void dbgListWindows(std::vector<std::string>& dst) override;
};


static void SelectControlsByTypeNameImpl(System::Windows::DependencyObject^ obj, String^ name, List<System::Windows::DependencyObject^>^ dst)
{
    if (obj->GetType()->FullName == name)
        dst->Add(obj);

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        SelectControlsByTypeNameImpl(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), name, dst);
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(System::Windows::DependencyObject^ obj, String^ name)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    SelectControlsByTypeNameImpl(obj, name, ret);
    return ret;
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(String^ name)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows)
            SelectControlsByTypeNameImpl(w, name, ret);
    }
    return ret;
}

static void EmulateClick(System::Windows::Controls::Button^ button)
{
    using namespace System::Windows::Automation;
    auto peer = gcnew Peers::ButtonAutomationPeer(button);
    auto invoke = (Provider::IInvokeProvider^)peer->GetPattern(Peers::PatternInterface::Invoke);
    invoke->Invoke();
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

void rtvr2Context::setTalker(int id)
{
    // todo
}

void rtvr2Context::setVoiceParams(const rtvr2Params& v)
{
    // todo
}

void rtvr2Context::setStyleParams(const rtvr2Style& v)
{
    // todo
}

bool rtvr2Context::setupControls()
{
    if (m_tb_text)
        return true;

    auto tev = SelectControlsByTypeName("AI.Talk.Editor.TextEditView");
    if (tev->Count == 0)
        return false;

    auto tb = SelectControlsByTypeName(tev[0], "AI.Framework.Wpf.Controls.TextBoxEx");
    if (tb->Count == 0)
        return false;

    m_tb_text = (System::Windows::Controls::TextBox^)tb[0];

    auto buttons = SelectControlsByTypeName(tev[0], "System.Windows.Controls.Button");
    if (buttons->Count == 0)
        return false;

    m_bu_play = (System::Windows::Controls::Button^)buttons[0];
    if (buttons->Count > 4)
        m_bu_save = (System::Windows::Controls::Button^)buttons[4];

    return true;
}

bool rtvr2Context::talk(const std::string & text)
{
    if (!m_tb_text || !m_bu_play) {
        if (!setupControls())
            return false;
    }
    m_tb_text->Text = gcnew String(text.c_str());
    EmulateClick(m_bu_play);
    return true;
}



rtvr2IController::~rtvr2IController()
{
}

rtvr2Controller::~rtvr2Controller()
{
}

void rtvr2Controller::setTalker(int v)
{
    return rtvr2Context::getInstance()->setTalker(v);
}

void rtvr2Controller::setVoiceParams(const rtvr2Params& v)
{
    return rtvr2Context::getInstance()->setVoiceParams(v);
}

void rtvr2Controller::setStyleParams(const rtvr2Style& v)
{
    return rtvr2Context::getInstance()->setStyleParams(v);
}

bool rtvr2Controller::talk(const std::string & text)
{
    return rtvr2Context::getInstance()->talk(text);
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
