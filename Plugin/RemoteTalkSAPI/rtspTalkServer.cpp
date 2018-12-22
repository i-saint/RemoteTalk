#include "pch.h"
#include "rtspHookHandler.h"
#include "rtspTalkServer.h"

namespace rtsp {

TalkServer::TalkServer()
{
    HRESULT hr = m_voice.CoCreateInstance(CLSID_SpVoice);
    if (FAILED(hr)) {
        return;
    }

    rt::OverrideWaveOutIAT(::GetModuleHandleA("sapi.dll"));
    auto& wo = WaveOutHandler::getInstance();
    rt::AddWaveOutHandler(&wo);
    wo.onUpdate = [](rt::AudioData& ad) { TalkServer::getInstance().onUpdateBuffer(ad); };


    CComPtr<IEnumSpObjectTokens> cpEnum;
    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
    if (FAILED(hr)) {
        return;
    }

    ULONG n;
    hr = cpEnum->GetCount(&n);
    int id_seed = 0;
    while (SUCCEEDED(hr) && n--)
    {
        CComPtr<ISpObjectToken> cpVoiceToken;
        hr = cpEnum->Next(1, &cpVoiceToken, NULL);
        if (SUCCEEDED(hr)) {
            PWSTR str;
            SpGetDescription(cpVoiceToken, &str);
            auto desc = rt::ToMBS(str);
            m_voice_tokens.push_back(cpVoiceToken);

            rt::CastInfo cas{ id_seed++, desc };
            cas.params.push_back({ "Volume", 1.0f, 0.0f, 1.0f });
            cas.params.push_back({ "Rate", 1.0f, 0.0f, 2.0f });
            m_casts.push_back(std::move(cas));
        }
    }
}

TalkServer::~TalkServer()
{
    wait();
}

void TalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
    processMessages();
}


bool TalkServer::isReady()
{
    if (!m_voice)
        return false;
    return true;
}

TalkServer::Status TalkServer::onStats(StatsMessage& mes)
{
    if (!m_voice)
        return Status::Failed;

    mes.stats.params.cast = 0;

    USHORT volume;
    long rate;
    m_voice->GetVolume(&volume);
    m_voice->GetRate(&rate);
    mes.stats.params[0] = (float)volume / 100.0f; // [0, 100] -> [0.0, 1.0]
    mes.stats.params[1] = (float)(rate + 10) / 10.0f; // [-10, 10] -> [0.0, 2.0]

    mes.stats.host = "Windows SAPI";
    mes.stats.protocol_version = rtProtocolVersion;
    mes.stats.plugin_version = rtPluginVersion;
    mes.stats.casts = m_casts;
    return Status::Succeeded;
}

TalkServer::Status TalkServer::onTalk(TalkMessage& mes)
{
    if (!m_voice)
        return Status::Failed;

    wait();

    m_params = mes.params;
    WaveOutHandler::getInstance().mute = m_params.mute;

    m_voice->SetVoice(m_voice_tokens[mes.params.cast]);
    if (mes.params.isSet(0))
        m_voice->SetVolume((USHORT)(mes.params[0] * 100.0f));
    if (mes.params.isSet(1))
        m_voice->SetRate((long)(mes.params[0] * 10.0f - 10.0f));

    auto text = rt::ToWCS(rt::ToUTF8(mes.text));
    m_task_talk = std::async(std::launch::async, [this, text]() {
        m_playing = true;
        m_voice->Speak(text.c_str(), 0, nullptr);

        rt::AudioData terminator;
        onUpdateBuffer(terminator);

        m_playing = false;
    });

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
    if (!m_voice)
        return Status::Failed;

    m_voice->Pause();
    return Status::Succeeded;
}

#ifdef rtDebug
TalkServer::Status TalkServer::onDebug(DebugMessage& mes)
{
    return Status::Succeeded;
}
#endif

void TalkServer::wait()
{
    if (m_task_talk.valid())
        m_task_talk.wait();
}

void TalkServer::onUpdateBuffer(const rt::AudioData& data)
{
    if (!m_playing)
        return;

    auto tmp = std::make_shared<rt::AudioData>(data);
    if (m_params.force_mono)
        tmp->convertToMono();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}

} // namespace rtsp
