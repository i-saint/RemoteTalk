#include "pch.h"
#include "rtFoundation.h"
#include "rtAudioData.h"
#include "rtNorm.h"
#include "rtSerialization.h"

#ifdef rtEnableOgg
#ifdef _MSC_VER
    #pragma comment(lib, "libvorbis_static.lib")
    #pragma comment(lib, "libogg_static.lib")
#endif
#include "vorbis/vorbisenc.h"
#endif


namespace rt {

int SizeOf(AudioFormat f)
{
    int ret = 0;
    switch (f) {
    case AudioFormat::U8: ret = 1; break;
    case AudioFormat::S16: ret = 2; break;
    case AudioFormat::S24: ret = 3; break;
    case AudioFormat::S32: ret = 4; break;
    case AudioFormat::F32: ret = 4; break;
    default: break;
    }
    return ret;
}
int GetNumBits(AudioFormat f)
{
    return SizeOf(f) * 8;
}



AudioDataPtr AudioData::create(std::istream & is)
{
    auto ret = std::make_shared<AudioData>();
    ret->deserialize(is);
    return ret;
}

AudioData::AudioData()
{
}

AudioData::~AudioData()
{
}

#define EachMember(Body) Body(format) Body(frequency) Body(channels) Body(data)

void AudioData::serialize(std::ostream& os) const
{
#define Body(N) write(os, N);
    EachMember(Body)
#undef Body
}

void AudioData::deserialize(std::istream& is)
{
#define Body(N) read(is, N);
    EachMember(Body)
#undef Body
}
#undef EachMember

uint64_t AudioData::hash() const
{
    return gen_hash(data);
}

void AudioData::clear()
{
    format = AudioFormat::Unknown;
    frequency = 0;
    channels = 0;
    data.clear();
}

void* AudioData::allocateByte(size_t num)
{
    data.resize(num);
    return data.data();
}

void* AudioData::allocateSample(size_t num_samples)
{
    data.resize(SizeOf(format) * num_samples);
    return data.data();
}

size_t AudioData::getSampleLength() const
{
    auto s = SizeOf(format);
    return s > 0 ? data.size() / s : 0;
}

double AudioData::getDuration() const
{
    return (double)getSampleLength() / (frequency * channels);
}

void AudioData::convertToMono()
{
    if (channels == 1)
        return; // nothing todo

    int len = (int)getSampleLength();
    if (len == 0)
        return; // data is empty or unknown format

    int c = channels;
    auto convert = [len, c](auto *dst) {
        for (int i = 0; i*c < len; ++i)
            dst[i] = dst[i*c];
    };

    switch (format) {
    case AudioFormat::U8:  convert(get<unorm8n>()); break;
    case AudioFormat::S16: convert(get<snorm16>()); break;
    case AudioFormat::S24: convert(get<snorm24>()); break;
    case AudioFormat::S32: convert(get<snorm32>()); break;
    case AudioFormat::F32: convert(get<float>()); break;
    default: return;
    }
    data.resize(data.size() / c);
    channels = 1;
}

void AudioData::increaseChannels(int n)
{
    if (channels != 1 || channels == n)
        return; // must be mono before convert

    int ch = channels;
    int len = (int)getSampleLength() / ch;
    if (len == 0)
        return; // data is empty or unknown format

    data.resize(data.size() * n);

    auto convert = [len, ch, n](auto *dst) {
        int i = len;
        while (i--) {
            for (int ci = 0; ci < n; ++ci)
                dst[i*n + ci] = dst[i];
        }
    };

    switch (format) {
    case AudioFormat::U8:  convert(get<unorm8n>()); break;
    case AudioFormat::S16: convert(get<snorm16>()); break;
    case AudioFormat::S24: convert(get<snorm24>()); break;
    case AudioFormat::S32: convert(get<snorm32>()); break;
    case AudioFormat::F32: convert(get<float>()); break;
    default: return;
    }
    channels = n;
}

double AudioData::resample(AudioData& dst, int new_frequency, int new_length, double pos) const
{
    if (getSampleLength() == 0)
        return 0.0;

    const auto& src = *this;
    dst.format = src.format;
    dst.channels = src.channels;
    dst.frequency = new_frequency;
    dst.allocateSample(new_length);

    double step = (double)(frequency - 1) / (double)(new_frequency - 1);
    int src_len = (int)src.getSampleLength() / src.channels;
    int dst_len = (int)dst.getSampleLength() / src.channels;
    int ch = channels;

    auto convert = [step, src_len, dst_len, ch, pos](auto *dst, auto *src) {
        for (int i = 0; i < dst_len; ++i) {
            double sp = (double)i * step + pos;
            int si = (int)sp;
            float st = (float)frac(sp);
            if (sp < 0.0) {
                for (int ci = 0; ci < ch; ++ci)
                    dst[i*ch + ci] = src[ci];
            }
            else if (si < src_len - 1) {
                for (int ci = 0; ci < ch; ++ci)
                    dst[i*ch + ci] = lerp((float)src[si * ch + ci], (float)src[(si + 1) * ch + ci], st);
            }
            else {
                for (int ci = 0; ci < ch; ++ci)
                    dst[i*ch + ci] = src[(src_len - 1) * ch + ci];
            }
        }
    };
    switch (format) {
    case AudioFormat::U8:  convert(dst.get<unorm8n>(), src.get<unorm8n>()); break;
    case AudioFormat::S16: convert(dst.get<snorm16>(), src.get<snorm16>()); break;
    case AudioFormat::S24: convert(dst.get<snorm24>(), src.get<snorm24>()); break;
    case AudioFormat::S32: convert(dst.get<snorm32>(), src.get<snorm32>()); break;
    case AudioFormat::F32: convert(dst.get<float>(), src.get<float>()); break;
    default: break;
    }
    return std::min(step * dst_len + pos, (double)src_len);
}

int AudioData::toFloat(float *dst, int pos, int len_orig, bool multiply)
{
    int sample_length = (int)getSampleLength();
    pos = std::min(pos, sample_length);
    if (len_orig < 0)
        len_orig = sample_length;
    int len = len_orig;
    len = std::min(len, sample_length - pos);

    auto convert = [dst, multiply](const auto *src, int n, int z) {
        if (multiply) {
            for (int i = 0; i < n; ++i)
                dst[i] *= src[i];
        }
        else {
            for (int i = 0; i < n; ++i)
                dst[i] = src[i];
        }
        for (int i = n; i < z; ++i)
            dst[i] = 0.0f;
    };

    switch (format) {
    case AudioFormat::U8:  convert(get<unorm8n>() + pos, len, len_orig); break;
    case AudioFormat::S16: convert(get<snorm16>() + pos, len, len_orig); break;
    case AudioFormat::S24: convert(get<snorm24>() + pos, len, len_orig); break;
    case AudioFormat::S32: convert(get<snorm32>() + pos, len, len_orig); break;
    case AudioFormat::F32: convert(get<float>() + pos, len, len_orig); break;
    default: return 0;
    }
    return len;
}

double AudioData::resampleFloat(float *dst, int new_frequency, int new_channels, int length, double pos)
{
    AudioData tmp;
    auto ret = resample(tmp, new_frequency, length / new_channels, pos);
    tmp.increaseChannels(new_channels);
    tmp.toFloat(dst, 0, -1, true);
    return ret;
}

AudioData& AudioData::operator+=(const AudioData& v)
{
    if (format == AudioFormat::RawFile || v.data.empty() || v.format == AudioFormat::Unknown || v.format == AudioFormat::RawFile)
        return *this;

    if (format == AudioFormat::Unknown) {
        *this = v;
    }
    else if (channels == v.channels && frequency == v.frequency) {
        if (format == v.format) {
            data.insert(data.end(), v.data.begin(), v.data.end());
        }
        else {
            auto convert = [](auto *dst, const auto *src, size_t n) {
                for (size_t i = 0; i < n; ++i)
                    dst[i] = (float)src[i];
            };

            auto pos = data.size();
            allocateSample(getSampleLength() + v.getSampleLength());

#define Impl(SrcT)\
                switch (v.format) {\
                case AudioFormat::U8: convert((SrcT*)&data[pos], (const unorm8n*)&v.data[0], v.data.size() / sizeof(unorm8n)); break;\
                case AudioFormat::S16: convert((SrcT*)&data[pos], (const snorm16*)&v.data[0], v.data.size() / sizeof(snorm16)); break;\
                case AudioFormat::S24: convert((SrcT*)&data[pos], (const snorm24*)&v.data[0], v.data.size() / sizeof(snorm24)); break;\
                case AudioFormat::S32: convert((SrcT*)&data[pos], (const snorm32*)&v.data[0], v.data.size() / sizeof(snorm32)); break;\
                case AudioFormat::F32: convert((SrcT*)&data[pos], (const float*)&v.data[0], v.data.size() / sizeof(float)); break;\
                default: break;\
                }

            switch (format) {
            case AudioFormat::U8:
                Impl(unorm8n);
                break;
            case AudioFormat::S16:
                Impl(snorm16);
                break;
            case AudioFormat::S24:
                Impl(snorm24);
                break;
            case AudioFormat::S32:
                Impl(snorm32);
                break;
            case AudioFormat::F32:
                Impl(float);
                break;
            default:
                break;
            }
#undef Impl
        }
    }
    return *this;
}



struct WaveHeader
{
    char    RIFFTag[4] = { 'R', 'I', 'F', 'F' };
    int32_t nFileSize = 0;
    char    WAVETag[4] = { 'W', 'A', 'V', 'E' };
    char    fmtTag[4] = { 'f', 'm', 't', ' ' };
    int32_t nFmtSize = 16;
    int16_t shFmtID = 1;
    int16_t shCh = 2;
    int32_t nSampleRate = 48000;
    int32_t nBytePerSec = 96000;
    int16_t shBlockSize = 4;
    int16_t shBitPerSample = 16;
    char    dataTag[4] = { 'd', 'a', 't', 'a' };
    int32_t nBytesData = 0;
};

bool AudioData::exportWave(const std::wstring& path) const
{
    if (format == AudioFormat::RawFile || format == AudioFormat::F32)
        return false;

    std::ofstream os(path.c_str(), std::ios::binary);
    if (!os)
        return false;

    WaveHeader header;
    header.nSampleRate = frequency;
    header.shCh = (int16_t)channels;
    header.shBitPerSample = (int16_t)SizeOf(format) * 8;
    header.nBytePerSec = header.nSampleRate * header.shBitPerSample * header.shCh / 8;
    header.shBlockSize = header.shBitPerSample * header.shCh / 8;
    os.write((char*)&header, sizeof(header));
    os.write(data.data(), data.size());

    uint32_t total_size = (uint32_t)(data.size() + sizeof(WaveHeader));
    uint32_t filesize = total_size - 8;
    uint32_t datasize = total_size - 44;
    os.seekp(4);
    os.write((char*)&filesize, 4);
    os.seekp(40);
    os.write((char*)&datasize, 4);
    return true;
}


bool AudioData::exportOgg(const std::wstring& path, const OggSettings& settings) const
{
#ifdef rtEnableOgg
    std::ofstream os(path.c_str(), std::ios::binary);
    if (!os)
        return false;

    vorbis_info         vo_info;
    vorbis_comment      vo_comment;
    vorbis_dsp_state    vo_dsp;
    vorbis_block        vo_block;
    ogg_stream_state    og_stream;
    ogg_page            og_page;

    vorbis_info_init(&vo_info);
    switch (settings.bitrate_mode) {
    case BitrateMode::VBR:
        vorbis_encode_init(&vo_info, channels, frequency, -1, settings.target_bitrate, -1);
        break;
    default: // CBR
        vorbis_encode_init(&vo_info, channels, frequency, settings.target_bitrate, settings.target_bitrate, settings.target_bitrate);
        break;
    }
    vorbis_comment_init(&vo_comment);
    vorbis_analysis_init(&vo_dsp, &vo_info);
    vorbis_block_init(&vo_dsp, &vo_block);

    static int s_serial = 0;
    ogg_stream_init(&og_stream, ++s_serial);

    {
        ogg_packet og_header, og_header_comm, og_header_code;
        vorbis_analysis_headerout(&vo_dsp, &vo_comment, &og_header, &og_header_comm, &og_header_code);
        ogg_stream_packetin(&og_stream, &og_header);
        ogg_stream_packetin(&og_stream, &og_header_comm);
        ogg_stream_packetin(&og_stream, &og_header_code);

        // make ogg header data
        for (;;) {
            int result = ogg_stream_flush(&og_stream, &og_page);
            if (result == 0)
                break;
            os.write((char*)og_page.header, og_page.header_len);
            os.write((char*)og_page.body, og_page.body_len);
        }
    }

    RawVector<float> samples(getSampleLength());

    auto convert = [&samples](const auto *src) {
        auto *dst = samples.data();
        size_t n = samples.size();
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    };
    switch (format) {
    case AudioFormat::U8:  convert(get<unorm8n>()); break;
    case AudioFormat::S16: convert(get<snorm16>()); break;
    case AudioFormat::S24: convert(get<snorm24>()); break;
    case AudioFormat::S32: convert(get<snorm32>()); break;
    case AudioFormat::F32: convert(get<float>()); break;
    default: break;
    }

    int block_size = (int)samples.size() / channels;
    float **buffer = vorbis_analysis_buffer(&vo_dsp, block_size);
    for (int bi = 0; bi < block_size; ++bi) {
        for (int ci = 0; ci < channels; ++ci) {
            buffer[ci][bi] = samples[bi*channels + ci];
        }
    }

    auto page_out = [&]() {
        while (vorbis_analysis_blockout(&vo_dsp, &vo_block) == 1) {
            vorbis_analysis(&vo_block, nullptr);
            vorbis_bitrate_addblock(&vo_block);

            ogg_packet packet;
            while (vorbis_bitrate_flushpacket(&vo_dsp, &packet) == 1) {
                ogg_stream_packetin(&og_stream, &packet);
                for (;;) {
                    int result = ogg_stream_pageout(&og_stream, &og_page);
                    if (result == 0)
                        break;
                    os.write((char*)og_page.header, og_page.header_len);
                    os.write((char*)og_page.body, og_page.body_len);
                    if (ogg_page_eos(&og_page))
                        break;
                }
            }
        }
    };

    if (vorbis_analysis_wrote(&vo_dsp, block_size) == 0)
        page_out();
    if (vorbis_analysis_wrote(&vo_dsp, 0) == 0)
        page_out();

    ogg_stream_clear(&og_stream);
    vorbis_block_clear(&vo_block);
    vorbis_dsp_clear(&vo_dsp);
    vorbis_comment_clear(&vo_comment);
    vorbis_info_clear(&vo_info);
    return true;
#else
    return false;
#endif
}

} // namespace rt
