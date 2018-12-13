#include "pch.h"
#include "rtAudioFile.h"

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

bool ExportWave(const AudioData& ad, const std::wstring & path)
{
    if (ad.format == AudioFormat::RawFile || ad.format == AudioFormat::F32)
        return false;

    std::ofstream os(path.c_str(), std::ios::binary);
    if (!os)
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

bool ExportOgg(const AudioData& ad, const std::wstring & path, const OggSettings & settings)
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
        vorbis_encode_init(&vo_info, ad.channels, ad.frequency, -1, settings.target_bitrate, -1);
        break;
    default: // CBR
        vorbis_encode_init(&vo_info, ad.channels, ad.frequency, settings.target_bitrate, settings.target_bitrate, settings.target_bitrate);
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


    int block_size = (int)ad.getSampleLength() / ad.channels;
    float **buffer = vorbis_analysis_buffer(&vo_dsp, block_size);

    auto convert = [&ad, block_size, buffer](const auto *src) {
        for (int bi = 0; bi < block_size; ++bi) {
            for (int ci = 0; ci < ad.channels; ++ci)
                buffer[ci][bi] = src[bi*ad.channels + ci];
        }
    };
    switch (ad.format) {
    case AudioFormat::U8:  convert(ad.get<unorm8n>()); break;
    case AudioFormat::S16: convert(ad.get<snorm16>()); break;
    case AudioFormat::S24: convert(ad.get<snorm24>()); break;
    case AudioFormat::S32: convert(ad.get<snorm32>()); break;
    case AudioFormat::F32: convert(ad.get<float>()); break;
    default: break;
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
