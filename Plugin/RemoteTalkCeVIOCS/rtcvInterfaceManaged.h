#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace CeVIO::Talk::RemoteService;


ref class rtcvInterfaceManaged
{
public:
    static rtcvInterfaceManaged^ getInstance();

    rtcvInterfaceManaged();
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
    static rtcvInterfaceManaged s_instance;

    ref class CastInfo
    {
    public:
        int id;
        String^ name;
        List<String^>^ params;

        CastInfo(int i, String^ n) : id(i), name(n), params(gcnew List<String^>()) {}
    };
    List<CastInfo^>^ m_casts;
    int m_cast = 0;
    String^ m_text = "";

    Talker^ m_talker;
    SpeakingState^ m_state;
};

std::string ToStdString(String^ str);
