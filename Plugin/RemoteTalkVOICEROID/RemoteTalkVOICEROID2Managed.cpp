#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalk/rtTalkInterfaceImpl.h"
#include "RemoteTalk/rtSerialization.h"
#include "RemoteTalkVOICEROIDCommon.h"
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
    rt::CastList getCastList();

    bool getParams(rt::TalkParams& params);
    bool setCast(int v);
    bool setParams(const rt::TalkParams& params);
    bool setText(const char *text);

    bool stop();
    bool talk();

private:
    static rtvr2InterfaceManaged s_instance;

    bool setupControls();

    TextBox^ m_tb_text;
    Button^ m_bu_play;
    Button^ m_bu_stop;
    Button^ m_bu_rewind;
    ListView^ m_lv_casts;
    List<Slider^>^ m_sl_params = gcnew List<Slider^>();

    ref class CastInfo
    {
    public:
        int id;
        String^ name;
        List<String^>^ param_names;

        CastInfo(int i,  String^ n) : id(i), name(n), param_names(gcnew List<String^>){}
    };
    List<CastInfo^>^ m_casts;
};

class rtvr2TalkInterface : public rtvr2ITalkInterface
{
public:
    rtDefSingleton(rtvr2TalkInterface);

    rtvr2TalkInterface();
    ~rtvr2TalkInterface() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumCasts() const override;
    bool getCastInfo(int i, rt::CastInfo *dst) const override;
    bool setText(const char *text) override;

    bool ready() const override;
    bool talk(rt::TalkSampleCallback cb, void *userdata) override;
    bool stop() override;


    bool setCast(int v) override;

    bool prepareUI() override;
    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(const rt::AudioData& ad) override;

#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;
    std::atomic_bool m_is_playing{ false };
    rt::TalkSampleCallback m_sample_cb = nullptr;
    void *m_sample_cb_userdata = nullptr;
};


static void SelectControlsByTypeNameImpl(System::Windows::DependencyObject^ obj, String^ name, List<System::Windows::DependencyObject^>^ dst, bool one, bool recursive, int depth = 0)
{
    if (!obj || (!recursive && depth > 1))
        return;
    if (obj->GetType()->FullName == name) {
        dst->Add(obj);
        if (one)
            return;
    }

    int num_children = System::Windows::Media::VisualTreeHelper::GetChildrenCount(obj);
    for (int i = 0; i < num_children; i++)
        SelectControlsByTypeNameImpl(System::Windows::Media::VisualTreeHelper::GetChild(obj, i), name, dst, one, recursive, depth + 1);
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(System::Windows::DependencyObject^ obj, String^ name, bool one, bool recursive = true)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    SelectControlsByTypeNameImpl(obj, name, ret, one, recursive);
    return ret;
}

static List<System::Windows::DependencyObject^>^ SelectControlsByTypeName(String^ name, bool one, bool recursive = true)
{
    auto ret = gcnew List<System::Windows::DependencyObject^>();
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows)
            SelectControlsByTypeNameImpl(w, name, ret, one, recursive);
    }
    return ret;
}

static bool EmulateClick(Button^ button)
{
    if (!button)
        return true;
    if (!button->IsEnabled)
        return false;
    using namespace System::Windows::Automation;
    auto peer = gcnew Peers::ButtonAutomationPeer(button);
    auto invoke = (Provider::IInvokeProvider^)peer->GetPattern(Peers::PatternInterface::Invoke);
    invoke->Invoke();
    return true;
}

static void UpdateValue(Slider^ slider, double v)
{
    if (!slider)
        return;
    slider->Value = v;
}

static std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return rt::ToUTF8((const char*)ptr.ToPointer());
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
    // select "voice" tab
    if (tc->SelectedIndex != 1) {
        tc->SelectedIndex = 1;
        return false;
    }

    setupControls();
    return true;
}

rt::CastList rtvr2InterfaceManaged::getCastList()
{
    setupControls();

    rt::CastList ret;
    if (m_casts) {
        for each(auto ti in m_casts) {
            rt::CastInfoImpl ci;
            ci.id = ti->id;
            ci.name = ToStdString(ti->name);
            for each(auto pname in ti->param_names)
                ci.param_names.push_back(ToStdString(pname));
            ret.push_back(std::move(ci));
        }
    }
    return ret;
}

bool rtvr2InterfaceManaged::getParams(rt::TalkParams& params)
{
    if (m_lv_casts)
        params.cast = m_lv_casts->SelectedIndex;

    int n = std::min(m_sl_params->Count, rt::TalkParams::MaxParams);
    for (int i = 0; i < n; ++i)
        params[i] = (float)m_sl_params[i]->Value;
    return true;
}

bool rtvr2InterfaceManaged::setCast(int v)
{
    if (!m_lv_casts || v < 0 && v >= m_lv_casts->Items->Count)
        return false;
    m_lv_casts->SelectedIndex = v;
    return false;
}

bool rtvr2InterfaceManaged::setParams(const rt::TalkParams& params)
{
    if (m_lv_casts)
        setCast(params.cast);

    for (int i = 0; i < rt::TalkParams::MaxParams; ++i) {
        if(params.isSet(i) && i < m_sl_params->Count)
            UpdateValue(m_sl_params[i], params[i]);
    }

    return true;
}

bool rtvr2InterfaceManaged::setText(const char *text)
{
    if (!m_tb_text)
        return true;
    m_tb_text->Text = gcnew String(text);
    return true;
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
                m_lv_casts = dynamic_cast<ListView^>(listview[0]);

                m_casts = gcnew List<CastInfo^>();
                int index = 0;
                auto items = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListViewItem", false);
                for each(ListViewItem^ item in items) {
                    auto tbs = SelectControlsByTypeName(item, "System.Windows.Controls.TextBlock", false);
                    if (tbs->Count >= 2) {
                        auto tb = dynamic_cast<TextBlock^>(tbs[1]);
                        m_casts->Add(gcnew CastInfo(index++, tb->Text));
                    }
                }
            }
        }
    }

    {
        auto vpev = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetEditView", true);
        if (vpev->Count >= 1) {
            auto cast = m_casts[m_lv_casts->SelectedIndex];
            cast->param_names->Clear();
            m_sl_params->Clear();

            auto lfs = SelectControlsByTypeName(vpev[0], "AI.Framework.Wpf.Controls.LinearFader", false);
            for (int lfi = 0; lfi < lfs->Count; ++lfi) {
                Slider^ slider = nullptr;
                TextBlock^ tblock = nullptr;

                {
                    auto c = SelectControlsByTypeName(lfs[lfi], "System.Windows.Controls.Slider", true);
                    if (c->Count >= 1)
                        slider = dynamic_cast<Slider^>(c[0]);
                }
                {
                    auto g = SelectControlsByTypeName(lfs[lfi], "System.Windows.Controls.Grid", true);
                    if (g->Count > 0) {
                        auto c = SelectControlsByTypeName(g[0], "System.Windows.Controls.TextBlock", true, false);
                        if (c->Count >= 1)
                            tblock = dynamic_cast<TextBlock^>(c[0]);
                    }
                }

                if (!slider || !tblock || !slider->IsEnabled)
                    continue;
                cast->param_names->Add(tblock->Text);
                m_sl_params->Add(slider);
            }
        }
    }
    return true;
}


bool rtvr2InterfaceManaged::stop()
{
    return EmulateClick(m_bu_stop);
}

bool rtvr2InterfaceManaged::talk()
{
    return EmulateClick(m_bu_rewind) && EmulateClick(m_bu_play);
}



rtvr2TalkInterface::rtvr2TalkInterface()
{
}

rtvr2TalkInterface::~rtvr2TalkInterface()
{
    auto mod = ::GetModuleHandleA("RemoteTalkVOICEROID2Hook.dll");
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtvr2TalkInterface::release() { /*do nothing*/ }
const char* rtvr2TalkInterface::getClientName() const { return "VOICEROID2"; }
int rtvr2TalkInterface::getPluginVersion() const { return rtPluginVersion; }
int rtvr2TalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool rtvr2TalkInterface::getParams(rt::TalkParams& params) const
{
    return rtvr2InterfaceManaged::getInstance()->getParams(params);
}

bool rtvr2TalkInterface::setParams(const rt::TalkParams& params)
{
    return rtvr2InterfaceManaged::getInstance()->setParams(params);
}

int rtvr2TalkInterface::getNumCasts() const
{
    m_casts = rtvr2InterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

bool rtvr2TalkInterface::getCastInfo(int i, rt::CastInfo *dst) const
{
    if (i >= 0 && i < (int)m_casts.size()) {
        *dst = m_casts[i].toCastInfo();
        return true;
    }
    return false;
}

bool rtvr2TalkInterface::setText(const char *text)
{
    return rtvr2InterfaceManaged::getInstance()->setText(text);
}

bool rtvr2TalkInterface::ready() const
{
    return !m_is_playing;
}

bool rtvr2TalkInterface::talk(rt::TalkSampleCallback cb, void *userdata)
{
    if (m_is_playing)
        return false;

    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtvr2InterfaceManaged::getInstance()->talk()) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool rtvr2TalkInterface::stop()
{
    if (!m_is_playing)
        return false;
    return rtvr2InterfaceManaged::getInstance()->stop();
}


bool rtvr2TalkInterface::setCast(int v)
{
    return rtvr2InterfaceManaged::getInstance()->setCast(v);
}

bool rtvr2TalkInterface::prepareUI()
{
    return rtvr2InterfaceManaged::getInstance()->prepareUI();
}

void rtvr2TalkInterface::onPlay()
{
    m_is_playing = true;
}

void rtvr2TalkInterface::onStop()
{
    if (m_sample_cb && m_is_playing) {
        rt::AudioData dummy;
        auto sd = rt::ToTalkSample(dummy);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
    m_is_playing = false;
}

void rtvr2TalkInterface::onUpdateBuffer(const rt::AudioData& ad)
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

bool rtvr2TalkInterface::onDebug()
{
    if (System::Windows::Application::Current != nullptr) {
        for each(System::Windows::Window^ w in System::Windows::Application::Current->Windows) {
            PrintControlInfo(w);
        }
    }
    return true;
}
#endif


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2TalkInterface::getInstance();
}
