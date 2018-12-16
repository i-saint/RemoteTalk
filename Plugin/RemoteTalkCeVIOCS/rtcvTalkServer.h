#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

class rtcvTalkServer : public rt::TalkServer
{
    using super = rt::TalkServer;
public:
    rtDefSingleton(rtcvTalkServer);
    rtcvTalkServer();
    void addMessage(MessagePtr mes) override;

    bool ready() override;
    Status onStats(StatsMessage& mes) override;
    Status onTalk(TalkMessage& mes) override;
    Status onStop(StopMessage& mes) override;
#ifdef rtDebug
    Status onDebug(DebugMessage& mes) override;
#endif

    static void sampleCallbackS(const rt::TalkSample *data, void *userdata);
    void sampleCallback(const rt::TalkSample *data);

private:
    rt::TalkParams m_params;
    std::future<void> m_task_talk;
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;
};
