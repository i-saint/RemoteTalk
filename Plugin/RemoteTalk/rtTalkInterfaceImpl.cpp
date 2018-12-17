#include "pch.h"
#include "rtTalkInterfaceImpl.h"
#include "rtAudioData.h"
#include "picojson/picojson.h"

namespace rt {

TalkSample ToTalkSample(const AudioData& ad)
{
    TalkSample ret;
    ret.data = ad.data.data();
    ret.size = (int)ad.data.size();
    ret.bits = rt::GetBitCount(ad.format);
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

void CastInfoImpl::fromCastInfo(const CastInfo& src)
{
    id = src.id;
    name = src.name;

    param_names.resize(src.num_ex_params);
    for (int i = 0; i < src.num_ex_params; ++i)
        param_names[i] = src.ex_param_names[i];
}

CastInfo CastInfoImpl::toCastInfo()
{
    static std::vector<const char*> s_pnames;

    CastInfo dst;
    s_pnames.resize(param_names.size());
    for (size_t i = 0; i < param_names.size(); ++i)
        s_pnames[i] = param_names[i].c_str();

    dst.id = id;
    dst.name = name.c_str();
    dst.num_ex_params = (int)param_names.size();
    dst.ex_param_names = &s_pnames[0];
    return dst;
}

} // namespace rt
