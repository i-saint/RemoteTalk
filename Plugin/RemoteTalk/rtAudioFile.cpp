#include "pch.h"
#include <fstream>
#include "rtAudioFile.h"
#include "rtSerialization.h"

#ifdef rtEnableOgg
#ifdef _MSC_VER
    #pragma comment(lib, "libvorbis_static.lib")
    #pragma comment(lib, "libogg_static.lib")
#endif
#include "vorbis/vorbisenc.h"
#endif

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

bool ExportWave(const AudioData& ad, std::ostream& os)
{
    if (ad.channels == 0 || ad.format == AudioFormat::RawFile || ad.format == AudioFormat::F32)
        return false;

    WaveHeader header;
    header.nSampleRate = ad.frequency;
    header.shCh = (int16_t)ad.channels;
    header.shBitPerSample = (int16_t)SizeOf(ad.format) * 8;
    header.nBytePerSec = header.nSampleRate * header.shBitPerSample * header.shCh / 8;
    header.shBlockSize = header.shBitPerSample * header.shCh / 8;
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

bool ExportOgg(const AudioData& ad, std::ostream& os, const OggSettings& settings)
{
#ifdef rtEnableOgg
    if (ad.channels == 0 || ad.format == AudioFormat::RawFile)
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
        vorbis_encode_init(&vo_info, ad.channels, ad.frequency, -1, settings.target_bitrate, -1);
        break;
    default: // CBR
        vorbis_encode_init(&vo_info, ad.channels, ad.frequency, settings.target_bitrate, settings.target_bitrate, settings.target_bitrate);
        break;
    }
    vorbis_comment_init(&vo_comment);
    if (vorbis_analysis_init(&vo_dsp, &vo_info) != 0)
        goto bailout1;
    if (vorbis_block_init(&vo_dsp, &vo_block) != 0)
        goto bailout2;

    static int s_serial = 0;
    if (ogg_stream_init(&og_stream, ++s_serial) != 0)
        goto bailout3;

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


    int sample_len = (int)ad.getSampleLength() / ad.channels;

    auto convert_block = [&](const auto *src, int pos, int len) -> int {
        len = std::min(len, sample_len - pos);
        float **buffer = vorbis_analysis_buffer(&vo_dsp, len);
        for (int bi = 0; bi < len; ++bi) {
            for (int ci = 0; ci < ad.channels; ++ci)
                buffer[ci][bi] = src[(bi*ad.channels + ci) + (pos * ad.channels)];
        }
        return len;
    };

    auto page_out = [&](int len) {
        if (vorbis_analysis_wrote(&vo_dsp, len) == 0) {
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
        }
    };

    const int block_size = 4096;
    int sample_pos = 0;
    for (;;) {
        int len = 0;
        switch (ad.format) {
        case AudioFormat::U8:  len = convert_block(ad.get<unorm8n>(), sample_pos, block_size); break;
        case AudioFormat::S16: len = convert_block(ad.get<snorm16>(), sample_pos, block_size); break;
        case AudioFormat::S24: len = convert_block(ad.get<snorm24>(), sample_pos, block_size); break;
        case AudioFormat::S32: len = convert_block(ad.get<snorm32>(), sample_pos, block_size); break;
        case AudioFormat::F32: len = convert_block(ad.get<float>()  , sample_pos, block_size); break;
        default: break;
        }
        page_out(len);
        sample_pos += len;
        if (len == 0)
            break;
    }

    ogg_stream_clear(&og_stream);
bailout3:
    vorbis_block_clear(&vo_block);
bailout2:
    vorbis_dsp_clear(&vo_dsp);
bailout1:
    vorbis_comment_clear(&vo_comment);
    vorbis_info_clear(&vo_info);
    return true;
#else
    return false;
#endif
}

bool ExportOgg(const AudioData& ad, const char *path, const OggSettings& settings)
{
#if _WIN32
    auto wpath = ToWCS(path);
    std::ofstream os(wpath.c_str(), std::ios::binary);
#else
    std::ofstream os(path, std::ios::binary);
#endif
    if (!os)
        return false;
    return ExportOgg(ad, os, settings);
}

} // namespace rt
