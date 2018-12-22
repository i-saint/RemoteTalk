#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace CeVIO::Talk::RemoteService;

namespace rtcv {

ref class TalkInterfaceManaged
{
public:
    static TalkInterfaceManaged^ getInstance();

    TalkInterfaceManaged();
    void updateStats();
    void updateCast();

    rt::CastList getCastList();
    bool getParams(rt::TalkParams& params);
    bool setParams(const rt::TalkParams& params);
    bool setText(const char *text);

    bool talk();
    bool stop();
    bool wait();
    bool isPlaying();

private:
    static TalkInterfaceManaged s_instance;

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

        CastInfo(int i, String^ n) : id(i), name(n), params(gcnew List<ParamInfo^>()) {}
    };
    List<CastInfo^>^ m_casts;
    int m_cast = 0;
    String^ m_text = "";

    Talker^ m_talker;
    SpeakingState^ m_state;
};

std::string ToStdString(String^ str);

} // namespace rtcv
