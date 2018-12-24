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
    if (vorbis_encode_init_vbr(&vo_info, ad.channels, ad.frequency, settings.quality) != 0)
        return false;
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
    vorbis_block_clear(&vo_block);
    vorbis_dsp_clear(&vo_dsp);
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
