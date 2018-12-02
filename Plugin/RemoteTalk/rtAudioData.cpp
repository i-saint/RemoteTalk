#include "pch.h"
#include "rtAudioData.h"
#include "rtNorm.h"
#include "rtSerialization.h"

namespace rt {

static inline int SizeOf(AudioFormat f)
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


AudioData::AudioData()
{
}

AudioData::~AudioData()
{
}

void AudioData::serialize(std::ostream& os) const
{
    write(os, format);
    write(os, frequency);
    write(os, channels);
    write(os, data);
}

void AudioData::deserialize(std::istream& is)
{
    read(is, format);
    read(is, frequency);
    read(is, channels);
    read(is, data);
}

uint64_t AudioData::hash() const
{
    return uint64_t();
}

size_t AudioData::getSampleLength() const
{
    return data.size() / SizeOf(format);
}

double AudioData::getDuration() const
{
    return (double)getSampleLength() / (frequency * channels);
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

bool AudioData::exportAsWave(const char *path) const
{
    if (format == AudioFormat::RawFile || format == AudioFormat::F32)
        return false;

    std::ofstream os(path, std::ios::binary);
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

bool AudioData::convertSamplesToFloat(float *dst)
{
    if (format == AudioFormat::U8) {
        auto src = (const unorm8n*)data.data();
        auto n = data.size() / sizeof(unorm8n);
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    }
    else if (format == AudioFormat::S16) {
        auto src = (const snorm16*)data.data();
        auto n = data.size() / sizeof(snorm16);
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    }
    else if (format == AudioFormat::S24) {
        auto src = (const snorm24*)data.data();
        auto n = data.size() / sizeof(snorm24);
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    }
    else if (format == AudioFormat::S32) {
        auto src = (const snorm32*)data.data();
        auto n = data.size() / sizeof(snorm32);
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    }
    else if (format == AudioFormat::F32) {
        auto src = (const float*)data.data();
        auto n = data.size() / sizeof(float);
        for (size_t i = 0; i < n; ++i)
            dst[i] = src[i];
    }
    else {
        return false;
    }
    return true;
}

} // namespace rt
