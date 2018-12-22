#include "pch.h"
#include "rtvrexCommon.h"
#include "rtvrexHookHandler.h"
#include "rtvrexTalkServer.h"
#include "rtvrexInterface.h"


static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}

void rtvrexTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool rtvrexTalkServer::isReady()
{
    return false;
}

rtvrexTalkServer::Status rtvrexTalkServer::onStats(StatsMessage& mes)
{
    auto& ifs = rtvrexInterface::getInstance();
    ifs.setupControls();

    auto& stats = mes.stats;
    stats.host = ifs.getClientName();
    stats.plugin_version = ifs.getPluginVersion();
    stats.protocol_version = ifs.getProtocolVersion();

    ifs.getParams(mes.stats.params);
    mes.stats.casts.push_back(*ifs.getCastInfo(0));

    return Status::Succeeded;
}

rtvrexTalkServer::Status rtvrexTalkServer::onTalk(TalkMessage& mes)
{
    auto& ifs = rtvrexInterface::getInstance();
    if(ifs.isPlaying())
        return Status::Failed;

    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    ifs.setAudioCallback([this](const rt::AudioData& data) {
        auto tmp = std::make_shared<rt::AudioData>(data);
        {
            std::unique_lock<std::mutex> lock(m_data_mutex);
            m_data_queue.push_back(tmp);
        }
    });

    ifs.setupControls();

    rtvrDSoundHandler::getInstance().mute = mes.params.mute;

    ifs.setParams(mes.params);
    if (!ifs.setText(mes.text.c_str()))
        return Status::Failed;
    if (!ifs.play())
        return Status::Failed;

    mes.task = std::async(std::launch::async, [this, &mes]() {
        std::vector<rt::AudioDataPtr> tmp;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(m_data_mutex);
                tmp = m_data_queue;
                m_data_queue.clear();
            }

            for (auto& ad : tmp) {
                ad->serialize(*mes.respond_stream);
            }

            if (!tmp.empty() && tmp.back()->data.empty())
                break;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    return Status::Succeeded;
}

rtvrexTalkServer::Status rtvrexTalkServer::onStop(StopMessage& mes)
{
    auto& ifs = rtvrexInterface::getInstance();
    if (!ifs.isPlaying())
        return Status::Failed;

    ifs.setupControls();
    ifs.stop();
    return Status::Succeeded;
}

#ifdef rtDebug
rtvrexTalkServer::Status rtvrexTalkServer::onDebug(DebugMessage& mes)
{
    auto& ifs = rtvrexInterface::getInstance();
    ifs.setupControls();
    return Status::Succeeded;
}
#endif
