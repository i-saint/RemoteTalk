#include "pch.h"
#include "rtvrCommon.h"
#include "rtvrHookHandler.h"
#include "rtvrTalkServer.h"


static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}

void rtvrTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool rtvrTalkServer::isReady()
{
    return rtGetTalkInterface_()->isReady();
}

rtvrTalkServer::Status rtvrTalkServer::onStats(StatsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        RequestUpdate();
        return Status::Pending;
    }

    auto& stats = mes.stats;
    if (!ifs->getParams(stats.params)) {
        RequestUpdate();
        return Status::Pending;
    }
    {
        int n = ifs->getNumCasts();
        for (int i = 0; i < n; ++i)
            stats.casts.push_back(*ifs->getCastInfo(i));
    }
    stats.host = ifs->getClientName();
    stats.plugin_version = ifs->getPluginVersion();
    stats.protocol_version = ifs->getProtocolVersion();
    return Status::Succeeded;
}

rtvrTalkServer::Status rtvrTalkServer::onTalk(TalkMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        RequestUpdate();
        return Status::Pending;
    }
    if (ifs->stop()) {
        // need to wait until next message if stop() succeeded.
        RequestUpdate();
        return Status::Pending;
    }
    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());

    rtvrDSoundHandler::getInstance().mute = mes.params.mute;
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    if (!ifs->talk())
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

rtvrTalkServer::Status rtvrTalkServer::onStop(StopMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        RequestUpdate();
        return Status::Pending;
    }
    return ifs->stop() ? Status::Succeeded : Status::Failed;
}

#ifdef rtDebug
rtvrTalkServer::Status rtvrTalkServer::onDebug(DebugMessage& mes)
{
    return rtGetTalkInterface_()->onDebug() ? Status::Succeeded : Status::Failed;
}
#endif


void rtvrTalkServer::onUpdateSample(const rt::AudioData& data)
{
    if (!rtGetTalkInterface_()->isPlaying())
        return;

    auto tmp = std::make_shared<rt::AudioData>(data);
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}

void rtvrTalkServer::onStop()
{
    if (!rtGetTalkInterface_()->isPlaying())
        return;

    auto terminator = std::make_shared<rt::AudioData>();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(terminator);
    }
}
