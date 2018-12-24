#include "pch.h"
#include <fstream>
#include "rtAudioFile.h"
#include "rtSerialization.h"

namespace rt {

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


bool ImportWave(AudioData& ad, std::istream& is)
{
    WaveHeader header;
    memset(&header, 0, sizeof(header));
    is.read((char*)&header, sizeof(header));
    if (memcmp(header.RIFFTag, "RIFF", 4) != 0)
        return false;

    ad.frequency = header.nSampleRate;
    ad.channels = header.shCh;
    if (header.shFmtID == 3) {
        if (header.shBitPerSample == 32)
            ad.format = AudioFormat::F32;
    }
    else {
        switch (header.shBitPerSample) {
        case 8: ad.format = AudioFormat::U8; break;
        case 16: ad.format = AudioFormat::S16; break;
        case 24: ad.format = AudioFormat::S24; break;
        case 32: ad.format = AudioFormat::S32; break;
        }
    }
    is.read((char*)ad.allocateByte(header.nBytesData), header.nBytesData);
    return true;
}

bool ImportWave(AudioData& ad, const char *path)
{
#if _WIN32
    auto wpath = ToWCS(path);
    std::ifstream is(wpath.c_str(), std::ios::binary);
#else
    std::ifstream is(path, std::ios::binary);
#endif
    if (!is)
        return false;
    return ImportWave(ad, is);
}


bool ExportWave(const AudioData& ad, std::ostream& os)
{
    if (ad.channels == 0 || ad.format == AudioFormat::RawFile)
        return false;

    WaveHeader header;
    header.nSampleRate = ad.frequency;
    header.shCh = (int16_t)ad.channels;
    header.shBitPerSample = (int16_t)GetBitCount(ad.format);
    header.shBlockSize = (int16_t)(SizeOf(ad.format) * ad.channels);
    header.nBytePerSec = ad.frequency * header.shBlockSize;
    header.shFmtID = ad.format == AudioFormat::F32 ? 3 : 1;
    os.write((char*)&header, sizeof(header));
    os.write(ad.data.data(), ad.data.size());

    uint32_t total_size = (uint32_t)(ad.data.size() + sizeof(WaveHeader));
    uint32_t filesize = total_size - 8;
    uint32_t datasize = total_size - 44;
    os.seekp(4);
    os.write((char*)&filesize, 4);
    os.seekp(40);
    os.write((char*)&datasize, 4);
    return true;
}

bool ExportWave(const AudioData& ad, const char *path)
{
#if _WIN32
    auto wpath = ToWCS(path);
    std::ofstream os(wpath.c_str(), std::ios::binary);
#else
    std::ofstream os(path, std::ios::binary);
#endif
    if (!os)
        return false;
    return ExportWave(ad, os);
}

} // namespace rt
