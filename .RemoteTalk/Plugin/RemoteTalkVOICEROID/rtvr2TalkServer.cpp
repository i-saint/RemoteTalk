#include "pch.h"
#include "rtvr2Common.h"
#include "rtvr2HookHandler.h"
#include "rtvr2TalkServer.h"

namespace rtvr2 {

TalkServer::TalkServer()
{
    auto exe_path = rt::GetMainModulePath();
    auto config_path = rt::GetCurrentModuleDirectory() + "\\" + rtvr2ConfigFile;
    auto settings = rt::GetOrAddServerSettings(config_path, exe_path, rtvr2DefaultPort);
    m_settings.port = settings.port;
}

void TalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
}

bool TalkServer::isReady()
{
    return rtGetTalkInterface_()->isReady();
}

TalkServer::Status TalkServer::onStats(StatsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    if (!ifs->isMainWindowVisible())
        return Status::Failed;

    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next update.
        return Status::Pending;
    }

    m_num_casts = ifs->getNumCasts();
    if (m_num_casts == 0)
        return Status::Failed;

    if (m_current_cast < m_num_casts) {
        if (ifs->setCast(m_current_cast)) {
            ifs->prepareUI();
            ++m_current_cast;
        }
        return Status::Pending;
    }

    auto& stats = mes.stats;
    if (!ifs->getParams(stats.params)) {
        return Status::Pending;
    }
    for (int i = 0; i < m_num_casts; ++i) {
        auto& cast = *ifs->getCastInfo(i);
        stats.casts.push_back(cast);
        if (cast.params.size() == 0)
            m_current_cast = 0;
    }
    stats.host = ifs->getClientName();
    stats.plugin_version = ifs->getPluginVersion();
    stats.protocol_version = ifs->getProtocolVersion();
    return m_current_cast > 0 ? Status::Succeeded : Status::Failed;
}

TalkServer::Status TalkServer::onTalk(TalkMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->isMainWindowVisible() || ifs->isPlaying())
        return Status::Failed;

    if (!ifs->setCast(mes.params.cast) || !ifs->prepareUI())
        return Status::Pending;

    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());

    DSoundHandler::getInstance().mute = mes.params.mute;
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    if (!ifs->play())
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

TalkServer::Status TalkServer::onStop(StopMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->isMainWindowVisible() || !ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        return Status::Pending;
    }
    return ifs->stop() ? Status::Succeeded : Status::Failed;
}

#ifdef rtDebug
TalkServer::Status TalkServer::onDebug(DebugMessage& mes)
{
    return rtGetTalkInterface_()->onDebug() ? Status::Succeeded : Status::Failed;
}
#endif


void TalkServer::onUpdateSample(const rt::AudioData& data)
{
    if (!rtGetTalkInterface_()->isPlaying())
        return;

    auto tmp = std::make_shared<rt::AudioData>(data);
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}

void TalkServer::onStop()
{
    if (!rtGetTalkInterface_()->isPlaying())
        return;

    auto terminator = std::make_shared<rt::AudioData>();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(terminator);
    }
}

} // namespace rtvr2
