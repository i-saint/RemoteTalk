#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"

namespace rtcv {

TalkServer::TalkServer()
{
    auto exe_path = rt::GetMainModulePath();
    auto config_path = rt::GetCurrentModuleDirectory() + "\\" + rtcvConfigFile;
    auto settings = rt::GetOrAddServerSettings(config_path, exe_path, rtcvDefaultPort);
    m_settings.port = settings.port;
    m_tmp_path = rt::GetCurrentModuleDirectory() + "\\tmp.wav";
}

void TalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
    processMessages();
}

bool TalkServer::isReady()
{
    return false;
}

TalkServer::Status TalkServer::onStats(StatsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();

    auto& stats = mes.stats;
    ifs->getParams(stats.params);
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

TalkServer::Status TalkServer::onTalk(TalkMessage& mes)
{
    if (m_task_talk.valid()) {
        if (m_task_talk.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
            return Status::Failed;
    }

    m_params = mes.params;

    auto ifs = rtGetTalkInterface_();
    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());

    if (m_mode == Mode::ExportFile) {
        ifs->setTempFilePath(m_tmp_path.c_str());
        if (!ifs->play())
            return Status::Failed;

        auto data = std::make_shared<rt::AudioData>();
        if (!rt::ImportWave(*data, m_tmp_path.c_str()))
            return Status::Failed;

        std::remove(m_tmp_path.c_str());
        m_data_queue.push_back(data);
        m_data_queue.push_back(std::make_shared<rt::AudioData>());
    }
    else {
        ifs->setTempFilePath("");
        WaveOutHandler::getInstance().mute = m_params.mute;
        if (!ifs->play())
            Status::Failed;

        m_task_talk = std::async(std::launch::async, [this, ifs]() {
            ifs->wait();
            WaveOutHandler::getInstance().mute = false;
            {
                auto terminator = std::make_shared<rt::AudioData>();
                std::unique_lock<std::mutex> lock(m_data_mutex);
                m_data_queue.push_back(terminator);
            }
        });
    }

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
    auto ifs = rtGetTalkInterface_();
    return ifs->stop() ? Status::Succeeded : Status::Failed;
}

#ifdef rtDebug
TalkServer::Status TalkServer::onDebug(DebugMessage& mes)
{
    return rtGetTalkInterface_()->onDebug() ? Status::Succeeded : Status::Failed;
}
#endif

void TalkServer::onUpdateBuffer(const rt::AudioData& data)
{
    auto ifs = rtGetTalkInterface_();
    if (!ifs->isPlaying())
        return;

    auto tmp = std::make_shared<rt::AudioData>(data);
    if (m_params.force_mono)
        tmp->convertToMono();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}

} // namespace rtcv
