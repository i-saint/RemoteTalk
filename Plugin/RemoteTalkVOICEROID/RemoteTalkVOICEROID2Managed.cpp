#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalkVOICEROID2Controller.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;


ref class rtvr2InterfaceManaged
{
public:
    static rtvr2InterfaceManaged^ getInstance();

    void setVolume(float v);
    void setTalker(int id);

    bool talk(const char *text);

private:
    static rtvr2InterfaceManaged s_instance;

    bool setupControls();

    System::Windows::Controls::TextBox^ m_tb_text;
    System::Windows::Controls::Button^ m_bu_play;
    System::Windows::Controls::Button^ m_bu_save;
};

class rtvr2TalkInterfaceImpl : public rtvr2TalkInterface
{
public:
    rtDefSingleton(rtvr2TalkInterfaceImpl);

    rtvr2TalkInterfaceImpl();
    ~rtvr2TalkInterfaceImpl() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool hasParam(rt::TalkParamID pid) const override;
    void setParam(rt::TalkParamID pid, const void *value) override;
    void getParam(rt::TalkParamID pid, void *value) const override;
    int getNumTalkers() const override;
    void getTalkerInfo(int i, rt::TalkerInfo *dst) const override;

    bool talk(const char *text, SampleCallback cb, void *userdata) override;


    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(const rt::AudioData& ad) override;

    void dbgListWindows(std::vector<std::string>& dst);

private:
    bool m_is_playing = false;
    SampleCallback m_sample_cb = nullptr;
    void *m_sample_cb_userdata = nullptr;
};


static void SelectControlsByTypeNameImpl(System::Windows::DependencyObject^ obj, String^ name, List<System::Windows::DependencyObject^>^ dst, bool one)
{
    if (obj->GetType()->FullName == name) {
        dst->Add(obj);
        if (one)
            return;
    }

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        SelectControlsByTypeNameImpl(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), name, dst, one);
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(System::Windows::DependencyObject^ obj, String^ name, bool one)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    SelectControlsByTypeNameImpl(obj, name, ret, one);
    return ret;
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(String^ name, bool one)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows)
            SelectControlsByTypeNameImpl(w, name, ret, one);
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


rtvr2InterfaceManaged^ rtvr2InterfaceManaged::getInstance()
{
    return %s_instance;
}

void rtvr2InterfaceManaged::setVolume(float v)
{
    // todo
}

void rtvr2InterfaceManaged::setTalker(int id)
{
    // todo
}

bool rtvr2InterfaceManaged::setupControls()
{
    if (m_tb_text)
        return true;

    auto tev = SelectControlsByTypeName("AI.Talk.Editor.TextEditView", true);
    if (tev->Count == 0)
        return false;

    auto tb = SelectControlsByTypeName(tev[0], "AI.Framework.Wpf.Controls.TextBoxEx", true);
    if (tb->Count == 0)
        return false;

    m_tb_text = (System::Windows::Controls::TextBox^)tb[0];

    auto buttons = SelectControlsByTypeName(tev[0], "System.Windows.Controls.Button", false);
    if (buttons->Count == 0)
        return false;

    m_bu_play = (System::Windows::Controls::Button^)buttons[0];
    if (buttons->Count > 4)
        m_bu_save = (System::Windows::Controls::Button^)buttons[4];

    return true;
}

bool rtvr2InterfaceManaged::talk(const char *text)
{
    if (!m_tb_text || !m_bu_play) {
        if (!setupControls())
            return false;
    }
    m_tb_text->Text = gcnew String(text);
    EmulateClick(m_bu_play);
    return true;
}



rtvr2TalkInterfaceImpl::rtvr2TalkInterfaceImpl()
{
}

rtvr2TalkInterfaceImpl::~rtvr2TalkInterfaceImpl()
{
}

void rtvr2TalkInterfaceImpl::release()
{
}

const char* rtvr2TalkInterfaceImpl::getClientName() const
{
    return "VOICEROID2";
}

int rtvr2TalkInterfaceImpl::getPluginVersion() const
{
    return rtPluginVersion;
}

int rtvr2TalkInterfaceImpl::getProtocolVersion() const
{
    return rtProtocolVersion;
}

bool rtvr2TalkInterfaceImpl::hasParam(rt::TalkParamID pid) const
{
    switch (pid) {
    case rt::TalkParamID::Volume:
    case rt::TalkParamID::Speed:
    case rt::TalkParamID::Pitch:
    case rt::TalkParamID::Intonation:
    case rt::TalkParamID::Joy:
    case rt::TalkParamID::Anger:
    case rt::TalkParamID::Sorrow:
    case rt::TalkParamID::Talker:
        return true;
    default:
        return false;
    }
}

void rtvr2TalkInterfaceImpl::setParam(rt::TalkParamID pid, const void *value)
{
    // todo
}

void rtvr2TalkInterfaceImpl::getParam(rt::TalkParamID pid, void *value) const
{
    // todo
}

int rtvr2TalkInterfaceImpl::getNumTalkers() const
{
    // todo
    return 0;
}

void rtvr2TalkInterfaceImpl::getTalkerInfo(int i, rt::TalkerInfo *dst) const
{
    // todo
}

bool rtvr2TalkInterfaceImpl::talk(const char *text, SampleCallback cb, void *userdata)
{
    if (m_is_playing || !text)
        return false;
    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtvr2InterfaceManaged::getInstance()->talk(text)) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}



void rtvr2TalkInterfaceImpl::onPlay()
{
    m_is_playing = true;
}

void rtvr2TalkInterfaceImpl::onStop()
{
    if (m_sample_cb && m_is_playing) {
        rt::AudioData dummy;
        auto sd = rt::ToTalkSample(dummy);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
    m_is_playing = false;
}

void rtvr2TalkInterfaceImpl::onUpdateBuffer(const rt::AudioData& ad)
{
    if (m_sample_cb && m_is_playing) {
        auto sd = rt::ToTalkSample(ad);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
}


static void GetControlInfo(System::Windows::DependencyObject^ obj, std::vector<std::string>& dst)
{
    dst.push_back(ToStdString(obj->GetType()->FullName));

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        GetControlInfo(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), dst);
}
void rtvr2TalkInterfaceImpl::dbgListWindows(std::vector<std::string>& dst)
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            GetControlInfo(w, dst);
        }
    }
}

rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2TalkInterfaceImpl::getInstance();
}
