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

    rt::TalkServerStats stats;
    if (!client.stats(stats))
        return;

    rt::TalkParams params = stats.params;
    params.params[0] = 0.8f;
    std::string text = "hello voiceroid! this is a test of remote talk client.";

    rt::AudioData sequence;
    client.play(params, text, [&](const rt::AudioData& ad) {
        if (ad.data.empty())
            return;
        sequence += ad;
    });
    if (!sequence.data.empty()) {
        rt::ExportWave(sequence, "hello_voiceroid.wav");
    }
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
    default:
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

    rt::ExportWave(sum, "sum_s16.wav");
    {
        rt::AudioData fsum;
        sum.convertFormat(fsum, rt::AudioFormat::F32);
        rt::ExportWave(fsum, "sum_f32.wav");
    }
    {
        rt::OggSettings ogg;
        ogg.quality = 1.0f;
        rt::ExportOgg(sum, "sum_vbr_high.ogg", ogg);
    }
    {
        rt::OggSettings ogg;
        ogg.quality = 0.0f;
        rt::ExportOgg(sum, "sum_vbr_low.ogg", ogg);
    }
}

TestCase(rtAudioData_Convert)
{
    rt::AudioData data;
    data.format = rt::AudioFormat::F32;
    data.frequency = 10;
    data.channels = 1;

    const int Len = 10;
    auto *samples = (float*)data.allocateSample(Len);
    for (int i = 0; i < Len; ++i)
        samples[i] = (2.0f / (Len - 1)) * i - 1.0f;

    rt::AudioData data2 = data;
    data2.increaseChannels(2);

    rt::AudioData data3 = data2;
    data3.convertToMono();

    rt::AudioData data4;
    data.resample(data4, 50, 50);
    rt::AudioData data5;
    data2.resample(data5, 5, 20, -5.0);

    Expect(data3.data == data.data);
}

#ifdef _WIN32
class TestFileIOHandler : public rt::FileIOHandlerBase
{

};

TestCase(FileIOHandler)
{
    rt::InstallFileIOHook(rt::HookType::ATOverride);
    {
        std::ofstream fo("fileiohandler_test.txt", std::ios::binary);
        fo.write("foo", 3);
    }
}
#endif
