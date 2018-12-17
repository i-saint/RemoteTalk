#include "pch.h"
#include "rtspHookHandler.h"
#include "rtspTalkServer.h"


rtspTalkServer::rtspTalkServer()
{
    HRESULT hr = m_voice.CoCreateInstance(CLSID_SpVoice);
    if (FAILED(hr)) {
        return;
    }

    rt::OverrideWaveOutIAT(::GetModuleHandleA("sapi.dll"));
    auto& wo = rtspWaveOutHandler::getInstance();
    rt::AddWaveOutHandler(&wo);
    wo.onUpdate = [](rt::AudioData& ad) { rtspTalkServer::getInstance().onUpdateBuffer(ad); };


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

            rt::CastInfoImpl cas{ id_seed++, desc };
            cas.param_names.push_back("Volume");
            cas.param_names.push_back("Rate");
            m_casts.push_back(std::move(cas));
        }
    }
}

rtspTalkServer::~rtspTalkServer()
{
    wait();
}

void rtspTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
    processMessages();
}


bool rtspTalkServer::ready()
{
    if (!m_voice)
        return false;
    return true;
}

rtspTalkServer::Status rtspTalkServer::onStats(StatsMessage& mes)
{
    if (!m_voice)
        return Status::Failed;

    mes.stats.params.cast = 0;

    USHORT volume;
    long rate;
    m_voice->GetVolume(&volume);
    m_voice->GetRate(&rate);
    mes.stats.params[0] = (float)volume / 100.0f;
    mes.stats.params[1] = (float)(rate + 10) / 10.0f;

    mes.stats.host = "Windows SAPI";
    mes.stats.protocol_version = rtProtocolVersion;
    mes.stats.plugin_version = rtPluginVersion;
    mes.stats.casts = m_casts;
    return Status::Succeeded;
}

rtspTalkServer::Status rtspTalkServer::onTalk(TalkMessage& mes)
{
    if (!m_voice)
        return Status::Failed;

    wait();

    m_params = mes.params;
    rtspWaveOutHandler::getInstance().mute = m_params.mute;

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

rtspTalkServer::Status rtspTalkServer::onStop(StopMessage& mes)
{
    if (!m_voice)
        return Status::Failed;

    m_voice->Pause();
    return Status::Succeeded;
}

#ifdef rtDebug
rtspTalkServer::Status rtspTalkServer::onDebug(DebugMessage& mes)
{
    return Status::Succeeded;
}
#endif

void rtspTalkServer::wait()
{
    if (m_task_talk.valid())
        m_task_talk.wait();
}

void rtspTalkServer::onUpdateBuffer(const rt::AudioData& data)
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

