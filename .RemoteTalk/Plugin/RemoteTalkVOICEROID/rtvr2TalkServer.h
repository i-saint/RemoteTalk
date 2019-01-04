#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

namespace rtvr2 {

class TalkServer : public rt::TalkServer
{
    using super = rt::TalkServer;
public:
    rtDefSingleton(TalkServer);
    TalkServer();
    void addMessage(MessagePtr mes) override;

    bool isReady() override;
    Status onStats(StatsMessage& mes) override;
    Status onTalk(TalkMessage& mes) override;
    Status onStop(StopMessage& mes) override;
#ifdef rtDebug
    Status onDebug(DebugMessage& mes) override;
#endif

    void onUpdateSample(const rt::AudioData& data);
    void onStop();

private:
    int m_num_casts = 0;
    int m_current_cast = 0;

    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;
};

} // namespace rtvr2
