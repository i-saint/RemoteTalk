#pragma once
#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <sapi.h>
#include <sphelper.h>

#include "rtspCommon.h"

namespace rtsp {

class TalkServer : public rt::TalkServer
{
    using super = rt::TalkServer;
public:
    rtDefSingleton(TalkServer);
    TalkServer();
    ~TalkServer();
    void addMessage(MessagePtr mes) override;

    bool isReady() override;
    Status onStats(StatsMessage& mes) override;
    Status onTalk(TalkMessage& mes) override;
    Status onStop(StopMessage& mes) override;
#ifdef rtDebug
    Status onDebug(DebugMessage& mes) override;
#endif

    void wait();
    void onUpdateBuffer(const rt::AudioData& data);

private:
    rt::TalkParams m_params;
    std::atomic_bool m_playing{false};

    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;

    rt::CastList m_casts;
    std::vector<CComPtr<ISpObjectToken>> m_voice_tokens;
    CComPtr<ISpVoice> m_voice;

    std::future<void> m_task_talk;
};

} // namespace rtsp
