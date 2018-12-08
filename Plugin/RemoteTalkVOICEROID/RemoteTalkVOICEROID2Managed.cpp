#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalk/rtTalkInterface.h"
#include "RemoteTalkVOICEROID2Controller.h"
#include <atomic>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Controls;

ref class rtvr2InterfaceManaged
{
public:
    static rtvr2InterfaceManaged^ getInstance();

    bool prepareUI();
    std::vector<rt::TalkerInfoImpl> getTalkerList();

    void getParams(rt::TalkParams& params);
    void setParams(const rt::TalkParams& params);

    bool stop();
    bool talk(const rt::TalkParams& params, const char *text);

private:
    static rtvr2InterfaceManaged s_instance;

    bool setupControls();

    TextBox^ m_tb_text;
    Button^ m_bu_play;
    Button^ m_bu_stop;
    Button^ m_bu_rewind;
    ListView^ m_lv_talker;
    TextBox ^m_tb_volume, ^m_tb_speed, ^m_tb_pitch, ^m_tb_intonation, ^m_tb_joy, ^m_tb_anger, ^m_tb_sorrow;

    ref class TalkerInfo
    {
    public:
        int id;
        String^ name;

        TalkerInfo(int i,  String^ n) : id(i), name(n) {}
    };
    List<TalkerInfo^>^ m_talkers;
};

class rtvr2TalkInterfaceImpl : public rtvr2TalkInterface
{
public:
    rtDefSingleton(rtvr2TalkInterfaceImpl);

    rtvr2TalkInterfaceImpl();
    ~rtvr2TalkInterfaceImpl() override;
    void release() override;
    const char* getClientName() const override;

    void getParams(rt::TalkParams& params) const override;

    int getNumTalkers() const override;
    bool getTalkerInfo(int i, rt::TalkerInfo *dst) const override;

    bool talk(const rt::TalkParams& params, const char *text, rt::TalkSampleCallback cb, void *userdata) override;
    bool stop() override;
    bool ready() const override;

    bool prepareUI() override;
    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(const rt::AudioData& ad) override;

#ifdef rtDebug
    void onDebug() override;
#endif

private:
    mutable std::vector<rt::TalkerInfoImpl> m_talkers;
    std::atomic_bool m_is_playing{ false };
    rt::TalkSampleCallback m_sample_cb = nullptr;
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

static void EmulateClick(Button^ button)
{
    if (!button)
        return;
    using namespace System::Windows::Automation;
    auto peer = gcnew Peers::ButtonAutomationPeer(button);
    auto invoke = (Provider::IInvokeProvider^)peer->GetPattern(Peers::PatternInterface::Invoke);
    invoke->Invoke();
}

static void UpdateText(TextBox^ tb, String^ text)
{
    if (!tb)
        return;
    using namespace System::Windows::Automation;
    auto peer = gcnew Peers::TextBoxAutomationPeer(tb);
    auto value = (Provider::IValueProvider^)peer->GetPattern(Peers::PatternInterface::Value);
    value->SetValue(text);

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

bool rtvr2InterfaceManaged::prepareUI()
{
    auto ttcs = SelectControlsByTypeName("AI.Framework.Wpf.Controls.TitledTabControl", true);
    if (ttcs->Count < 2)
        return true;

    auto tc = dynamic_cast<TabControl^>(ttcs[1]);
    if (tc->SelectedIndex != 1) {
        tc->SelectedIndex = 1;
        return false;
    }

    setupControls();
    return true;
}

std::vector<rt::TalkerInfoImpl> rtvr2InterfaceManaged::getTalkerList()
{
    setupControls();

    std::vector<rt::TalkerInfoImpl> ret;
    if (m_talkers) {
        for each(auto ti in m_talkers)
            ret.push_back({ ti->id, ToStdString(ti->name) });
    }
    return ret;
}

void rtvr2InterfaceManaged::getParams(rt::TalkParams& params)
{
    // todo
}

void rtvr2InterfaceManaged::setParams(const rt::TalkParams& params)
{
    // todo
}

bool rtvr2InterfaceManaged::setupControls()
{
    if (!m_tb_text) {
        auto tev = SelectControlsByTypeName("AI.Talk.Editor.TextEditView", true);
        if (tev->Count == 0)
            return false;

        auto tb = SelectControlsByTypeName(tev[0], "AI.Framework.Wpf.Controls.TextBoxEx", true);
        if (tb->Count == 0)
            return false;

        m_tb_text = dynamic_cast<TextBox^>(tb[0]);

        auto buttons = SelectControlsByTypeName(tev[0], "System.Windows.Controls.Button", false);
        if (buttons->Count == 0)
            return false;

        m_bu_play = dynamic_cast<Button^>(buttons[0]);
        if (buttons->Count >= 1)
            m_bu_stop = dynamic_cast<Button^>(buttons[1]);
        if (buttons->Count >= 2)
            m_bu_rewind = dynamic_cast<Button^>(buttons[2]);

        auto vplv = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetListView", true);
        if (vplv->Count >= 1) {
            auto listview = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListView", true);
            if (listview->Count >= 1) {
                m_lv_talker = dynamic_cast<ListView^>(listview[0]);

                m_talkers = gcnew List<TalkerInfo^>();
                int index = 0;
                auto items = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListViewItem", false);
                for each(ListViewItem^ item in items) {
                    auto tbs = SelectControlsByTypeName(item, "System.Windows.Controls.TextBlock", false);
                    if (tbs->Count >= 2) {
                        auto tb = dynamic_cast<TextBlock^>(tbs[1]);
                        m_talkers->Add(gcnew TalkerInfo(index++, tb->Text));
                    }
                }
            }
        }
    }

    {
        auto vpev = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetEditView", true);
        if (vpev->Count >= 1) {
            auto lfs = SelectControlsByTypeName(vpev[0], "AI.Framework.Wpf.Controls.LinearFader", false);
            for (int lfi = 0; lfi < lfs->Count; ++lfi) {
                TextBox^ tb = nullptr;
                auto tbs = SelectControlsByTypeName(lfs[lfi], "AI.Framework.Wpf.Controls.TextBoxEx", true);
                if (tbs->Count >= 1)
                    tb = dynamic_cast<TextBox^>(tbs[0]);

                switch (lfi)
                {
                case 0: m_tb_volume = tb; break;
                case 1: m_tb_speed = tb; break;
                case 2: m_tb_pitch = tb; break;
                case 3: m_tb_intonation = tb; break;
                case 4: break;
                case 5: break;
                case 6: m_tb_joy = tb; break;
                case 7: m_tb_anger = tb; break;
                case 8: m_tb_sorrow = tb; break;
                default: break;
                }
            }
            if (lfs->Count < 6) {
                m_tb_joy = m_tb_anger = m_tb_sorrow = nullptr;
            }
        }
    }
    return true;
}


bool rtvr2InterfaceManaged::stop()
{
    if (!setupControls())
        return false;
    EmulateClick(m_bu_stop);
    return true;
}

bool rtvr2InterfaceManaged::talk(const rt::TalkParams& params, const char *text)
{
    if (params.flags.fields.talker && m_lv_talker)
        m_lv_talker->SelectedIndex = params.talker;

    if (params.flags.fields.volume && m_tb_volume)
        UpdateText(m_tb_volume, params.volume.ToString());
    if (params.flags.fields.speed && m_tb_speed)
        UpdateText(m_tb_speed, params.speed.ToString());
    if (params.flags.fields.pitch && m_tb_pitch)
        UpdateText(m_tb_pitch, params.pitch.ToString());
    if (params.flags.fields.intonation && m_tb_intonation)
        UpdateText(m_tb_intonation, params.intonation.ToString());
    if (params.flags.fields.joy && m_tb_joy)
        UpdateText(m_tb_joy, params.joy.ToString());
    if (params.flags.fields.anger && m_tb_anger)
        UpdateText(m_tb_anger, params.anger.ToString());
    if (params.flags.fields.sorrow && m_tb_sorrow)
        UpdateText(m_tb_sorrow, params.sorrow.ToString());

    if (!text || !m_tb_text)
        return false;

    m_tb_text->Text = gcnew String(text);
    EmulateClick(m_bu_rewind);
    EmulateClick(m_bu_play);
    return true;
}



rtvr2TalkInterfaceImpl::rtvr2TalkInterfaceImpl()
{
}

rtvr2TalkInterfaceImpl::~rtvr2TalkInterfaceImpl()
{
    auto mod = ::GetModuleHandleA("RemoteTalkVOICEROID2Hook.dll");
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtvr2TalkInterfaceImpl::release()
{
    // do nothing
}

const char* rtvr2TalkInterfaceImpl::getClientName() const
{
    return "VOICEROID2";
}

void rtvr2TalkInterfaceImpl::getParams(rt::TalkParams& params) const
{
    rtvr2InterfaceManaged::getInstance()->getParams(params);
}

int rtvr2TalkInterfaceImpl::getNumTalkers() const
{
    if (m_talkers.empty())
        m_talkers = rtvr2InterfaceManaged::getInstance()->getTalkerList();
    return (int)m_talkers.size();
}

bool rtvr2TalkInterfaceImpl::getTalkerInfo(int i, rt::TalkerInfo *dst) const
{
    if (i < (int)m_talkers.size()) {
        dst->id = m_talkers[i].id;
        dst->name = m_talkers[i].name.c_str();
        return true;
    }
    return false;
}

bool rtvr2TalkInterfaceImpl::talk(const rt::TalkParams& params, const char *text, rt::TalkSampleCallback cb, void *userdata)
{
    if (m_is_playing)
        return false;

    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtvr2InterfaceManaged::getInstance()->talk(params, text)) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool rtvr2TalkInterfaceImpl::stop()
{
    return rtvr2InterfaceManaged::getInstance()->stop();
}

bool rtvr2TalkInterfaceImpl::ready() const
{
    return !m_is_playing;
}


bool rtvr2TalkInterfaceImpl::prepareUI()
{
    return rtvr2InterfaceManaged::getInstance()->prepareUI();
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

void rtvr2TalkInterfaceImpl::onDebug()
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            PrintControlInfo(w);
        }
    }
}
#endif


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2TalkInterfaceImpl::getInstance();
}
