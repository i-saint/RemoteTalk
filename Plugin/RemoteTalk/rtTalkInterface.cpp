#include "pch.h"
#include "rtTalkInterface.h"
#include "rtAudioData.h"

namespace rt {

TalkSample ToTalkSample(const AudioData& ad)
{
    TalkSample ret;
    ret.data = ad.data.data();
    ret.size = (int)ad.data.size();
    ret.bits = rt::GetNumBits(ad.format);
    ret.channels = ad.channels;
    ret.frequency = ad.frequency;
    return ret;
}

void ToAudioData(AudioData& dst, const TalkSample& ts)
{
    dst.data.assign(ts.data, ts.data + ts.size);
    switch (ts.bits) {
    case 8: dst.format = AudioFormat::U8; break;
    case 16: dst.format = AudioFormat::S16; break;
    case 24: dst.format = AudioFormat::S24; break;
    case 32: dst.format = AudioFormat::S32; break;
    }
    dst.channels = ts.channels;
    dst.frequency = ts.frequency;
}

} // namespace rt
