#pragma once

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

        CastInfo(int i, String^ n) : id(i), name(n), param_names(gcnew List<String^>) {}
    };
    List<CastInfo^>^ m_casts;
};

std::string ToStdString(String^ str);
