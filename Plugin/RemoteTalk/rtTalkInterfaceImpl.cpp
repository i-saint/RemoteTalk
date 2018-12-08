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


template<> std::string to_json(const TalkParams& v)
{
    picojson::object r;
    if (v.flags.mute) r["mute"] = picojson::value(v.mute);
    if (v.flags.volume) r["volume"] = picojson::value(v.volume);
    if (v.flags.speed) r["speed"] = picojson::value(v.speed);
    if (v.flags.pitch) r["pitch"] = picojson::value(v.pitch);
    if (v.flags.intonation) r["intonation"] = picojson::value(v.intonation);
    if (v.flags.joy) r["joy"] = picojson::value(v.joy);
    if (v.flags.anger) r["anger"] = picojson::value(v.anger);
    if (v.flags.sorrow) r["sorrow"] = picojson::value(v.sorrow);
    if (v.flags.avator) r["avator"] = picojson::value((float)v.avator);

    picojson::value t(std::move(r));
    return t.serialize(true);
}

template<> void from_json(TalkParams& v, const std::string& json)
{
    picojson::value tmp;
    picojson::parse(tmp, json);

    if (tmp.is<picojson::object>()) {
        auto& obj = tmp.get<picojson::object>();
        for (auto& kvp : obj) {
            if      (kvp.first == "mute") { v.setMute(kvp.second.get<bool>() != 0); }
            else if (kvp.first == "volume") { v.setVolume(kvp.second.get<float>()); }
            else if (kvp.first == "speed") { v.setSpeed(kvp.second.get<float>()); }
            else if (kvp.first == "pitch") { v.setPitch(kvp.second.get<float>()); }
            else if (kvp.first == "intonation") { v.setIntonation(kvp.second.get<float>()); }
            else if (kvp.first == "joy") { v.setJoy(kvp.second.get<float>()); }
            else if (kvp.first == "anger") { v.setAnger(kvp.second.get<float>()); }
            else if (kvp.first == "sorrow") { v.setSorrow(kvp.second.get<float>()); }
            else if (kvp.first == "avator") { v.setAvator((int)kvp.second.get<float>()); }
        }
    }
}

template<> std::string to_json(const std::vector<AvatorInfoImpl>& v)
{
    picojson::array r;
    r.resize(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        picojson::object o;
        o["id"] = picojson::value((float)v[i].id);
        o["name"] = picojson::value(v[i].name);
        r[i] = picojson::value(o);
    }

    picojson::value t(std::move(r));
    return t.serialize(true);
}

template<> void from_json(std::vector<AvatorInfoImpl>& dst, const std::string& json)
{
    picojson::value tmp;
    picojson::parse(tmp, json);

    if (tmp.is<picojson::array>()) {
        auto& ary = tmp.get<picojson::array>();
        for (auto& e : ary) {
            auto& o = e.get<picojson::object>();
            AvatorInfoImpl ai;
            ai.id = (int)o["id"].get<float>();
            ai.name = o["name"].get<std::string>();
            dst.push_back(ai);
        }
    }
}

} // namespace rt
