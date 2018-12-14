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
    client.talk(params, text, [&](const rt::AudioData& ad) {
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

    rt::ExportWave(sum, "sum.wav");
    {
        rt::OggSettings ogg;
        rt::ExportOgg(sum, "sum_vbr.ogg", ogg);
    }
    {
        rt::OggSettings ogg;
        ogg.bitrate_mode = rt::BitrateMode::CBR;
        rt::ExportOgg(sum, "sum_cbr.ogg", ogg);
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

TestCase(LaunchVoiceroid2)
{
#ifdef _WIN32
    //C:\\Program Files (x86)\\AHS\VOICEROID2\\

    char install_dir[MAX_PATH+1];
    DWORD install_dir_size = sizeof(install_dir);
    if (::RegGetValueA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{6F962085-923C-4AEE-BFC2-D64FAEE9B82D}",
        "InstallLocation", RRF_RT_REG_SZ, NULL, install_dir, &install_dir_size) == ERROR_SUCCESS)
    {
        std::string exe_path(install_dir, install_dir_size - 1);
        exe_path += "VoiceroidEditor.exe";

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ::ZeroMemory(&si, sizeof(si));
        ::ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        BOOL ret = ::CreateProcessA(exe_path.c_str(), nullptr, nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS, nullptr, nullptr, &si, &pi);
        if (ret) {
        }
    }
#endif
}


#ifdef _WIN32
#import "libid:D3AEA482-B527-4818-8CEA-810AFFCB24B6" named_guids rename_namespace("CeVIO")
#endif

TestCase(LaunchCeVIOCS)
{
#ifdef _WIN32
    ::CoInitialize(NULL);

    // note: this always fails on x64
    CeVIO::IServiceControl *pServiceControl;
    HRESULT result0 = ::CoCreateInstance(CeVIO::CLSID_ServiceControl, NULL, CLSCTX_INPROC_SERVER, CeVIO::IID_IServiceControl, reinterpret_cast<LPVOID *>(&pServiceControl));
    if (FAILED(result0)) {
        return;
    }

    pServiceControl->StartHost(false);
#endif
}
