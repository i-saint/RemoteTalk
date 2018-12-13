#include "pch.h"
#include "rtFoundation.h"
#include "rtAudioData.h"
#include "rtNorm.h"
#include "rtSerialization.h"


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

} // namespace rt
