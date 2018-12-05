#include "pch.h"
#include "Test.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"


static rt::TalkClientSettings GetClientSettings()
{
    rt::TalkClientSettings ret;
    GetArg("server", ret.server);
    int port;
    if (GetArg("port", port))
        ret.port = (uint16_t)port;
    return ret;
}

TestCase(RemoteTalkClient)
{
    rt::TalkClient client(GetClientSettings());
    client.setText("hello voiceroid!");

    int n = 0;
    rt::AudioData sequence;
    client.send([&](const rt::AudioData& ad) {
        if (ad.data.empty())
            return;
        sequence += ad;
    });
    if (!sequence.data.empty())
        sequence.exportAsWave("hello_voiceroid.wav");
}


static const int Frequency = 48000;
static const int Channels = 1;

template<class T >
static void GenerateAudioSample(T *dst, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        float s = std::pow(float(n - i) / n, 0.5f);
        dst[i] = std::sin((float(i) * 1.5f * rt::Deg2Rad)) * s;
    }
}

static rt::AudioDataPtr CreateAudioAsset(rt::AudioFormat fmt)
{
    using namespace rt;

    auto a = std::make_shared<rt::AudioData>();
    a->format = fmt;
    a->frequency = Frequency;
    a->channels = Channels;

    auto samples = a->allocateSample(Frequency / 2); // 0.5 sec
    switch (fmt) {
    case AudioFormat::U8:
        GenerateAudioSample((unorm8n*)samples, a->getSampleLength() * Channels);
        break;
    case AudioFormat::S16:
        GenerateAudioSample((snorm16*)samples, a->getSampleLength() * Channels);
        break;
    case AudioFormat::S24:
        GenerateAudioSample((snorm24*)samples, a->getSampleLength() * Channels);
        break;
    case AudioFormat::S32:
        GenerateAudioSample((snorm32*)samples, a->getSampleLength() * Channels);
        break;
    case AudioFormat::F32:
        GenerateAudioSample((float*)samples, a->getSampleLength() * Channels);
        break;
    }
    return a;
}

TestCase(rtAudioData)
{
    auto adu8 = CreateAudioAsset(rt::AudioFormat::U8);
    auto ads16 = CreateAudioAsset(rt::AudioFormat::S16);
    auto ads24 = CreateAudioAsset(rt::AudioFormat::S24);
    auto ads32 = CreateAudioAsset(rt::AudioFormat::S32);
    auto adf32 = CreateAudioAsset(rt::AudioFormat::F32);

    rt::AudioData sum;
    sum.format = rt::AudioFormat::S16;
    sum.frequency = Frequency;
    sum.channels = Channels;

    sum += *adu8;
    sum += *ads16;
    sum += *ads24;
    sum += *ads32;
    sum += *adf32;

    sum.exportAsWave("sum.wav");
}
