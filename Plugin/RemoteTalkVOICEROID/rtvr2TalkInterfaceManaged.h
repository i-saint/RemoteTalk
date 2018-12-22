#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Windows::Controls;

namespace rtvr2 {

ref class TalkInterfaceManaged
{
public:
    static TalkInterfaceManaged^ getInstance();

    bool prepareUI();
    rt::CastList getCastList();

    bool getParams(rt::TalkParams& params);
    bool setCast(int v);
    bool setParams(const rt::TalkParams& params);
    bool setText(const char *text);

    bool stop();
    bool talk();

private:
    static TalkInterfaceManaged s_instance;

    bool setupControls();

    TextBox^ m_tb_text;
    Button^ m_bu_play;
    Button^ m_bu_stop;
    Button^ m_bu_rewind;
    ListView^ m_lv_casts;
    List<Slider^>^ m_sl_params = gcnew List<Slider^>();

    ref class ParamInfo
    {
    public:
        String^ name;
        float value = 0.0f, range_min = 0.0f, range_max = 0.0f;

        ParamInfo(String^ n, float v, float rn, float rx) : name(n), value(v), range_min(rn), range_max(rx) {}
    };

    ref class CastInfo
    {
    public:
        int id;
        String^ name;
        List<ParamInfo^>^ params;

        CastInfo(int i, String^ n) : id(i), name(n), params(gcnew List<ParamInfo^>) {}
    };
    List<CastInfo^>^ m_casts;
};

std::string ToStdString(String^ str);

} // namespace rtvr2
