#include "pch.h"
#include "rtvr2Common.h"
#include "rtvr2TalkInterfaceManaged.h"

namespace rtvr2 {

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

std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return rt::ToUTF8((const char*)ptr.ToPointer());
}


TalkInterfaceManaged^ TalkInterfaceManaged::getInstance()
{
    return %s_instance;
}

bool TalkInterfaceManaged::prepareUI()
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

rt::CastList TalkInterfaceManaged::getCastList()
{
    setupControls();

    rt::CastList ret;
    if (m_casts) {
        for each(auto casts in m_casts) {
            rt::CastInfo ci;
            ci.id = casts->id;
            ci.name = ToStdString(casts->name);
            for each(auto p in casts->params)
                ci.params.push_back({ ToStdString(p->name), p->value, p->range_min, p->range_max });
            ret.push_back(std::move(ci));
        }
    }
    return ret;
}

bool TalkInterfaceManaged::getParams(rt::TalkParams& params)
{
    if (m_lv_casts)
        params.cast = m_lv_casts->SelectedIndex;

    int n = std::min(m_sl_params->Count, rt::TalkParams::MaxParams);
    for (int i = 0; i < n; ++i)
        params[i] = (float)m_sl_params[i]->Value;
    return true;
}

bool TalkInterfaceManaged::setCast(int v)
{
    if (!m_lv_casts || v < 0 || v >= m_lv_casts->Items->Count)
        return false;
    if (m_lv_casts->SelectedIndex != v) {
        m_lv_casts->SelectedIndex = v;
        return false;
    }
    return true;
}

bool TalkInterfaceManaged::setParams(const rt::TalkParams& params)
{
    if (m_lv_casts)
        setCast(params.cast);

    for (int i = 0; i < rt::TalkParams::MaxParams; ++i) {
        if (params.isSet(i) && i < m_sl_params->Count)
            UpdateValue(m_sl_params[i], params[i]);
    }

    return true;
}

bool TalkInterfaceManaged::setText(const char *text)
{
    if (!m_tb_text)
        return true;
    m_tb_text->Text = gcnew String(text);
    return true;
}

bool TalkInterfaceManaged::setupControls()
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

        if (m_casts == nullptr) {
            auto vplv = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetListView", true);
            if (vplv->Count >= 1) {
                auto listview = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListView", true);
                if (listview->Count >= 1) {
                    m_lv_casts = dynamic_cast<ListView^>(listview[0]);

                    int index = 0;
                    auto items = SelectControlsByTypeName(vplv[0], "System.Windows.Controls.ListViewItem", false);

                    m_casts = gcnew List<CastInfo^>();
                    for each(ListViewItem^ item in items) {
                        auto tbs = SelectControlsByTypeName(item, "System.Windows.Controls.TextBlock", false);
                        if (tbs->Count >= 2) {
                            auto tb = dynamic_cast<TextBlock^>(tbs[1]);
                            m_casts->Add(gcnew CastInfo(index, tb->Text));
                            ++index;
                        }
                    }
                }
            }
        }
    }

    {
        auto vpev = SelectControlsByTypeName("AI.Talk.Editor.VoicePresetEditView", true);
        if (vpev->Count >= 1) {
            auto cast = m_casts[m_lv_casts->SelectedIndex];
            cast->params->Clear();
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

                m_sl_params->Add(slider);
                cast->params->Add(
                    gcnew ParamInfo(tblock->Text, (float)slider->Value, (float)slider->Minimum, (float)slider->Maximum));
            }
        }
    }
    return true;
}


bool TalkInterfaceManaged::stop()
{
    return EmulateClick(m_bu_stop);
}

bool TalkInterfaceManaged::talk()
{
    return EmulateClick(m_bu_rewind) && EmulateClick(m_bu_play);
}

} // namespace rtvr2
