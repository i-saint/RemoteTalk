#include "pch.h"
#include "rtSerialization.h"
#include "rtTalkInterfaceImpl.h"
#include "picojson/picojson.h"

namespace rt {

std::string ToUTF8(const char *src)
{
#ifdef _WIN32
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize);
    ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to UTF-8
    const int u8size = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size);
    ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
#else
    return src;
#endif
}
std::string ToUTF8(const std::string& src)
{
    return ToUTF8(src.c_str());
}

std::string ToANSI(const char *src)
{
#ifdef _WIN32
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to ANSI
    const int u8size = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size);
    ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
#else
    return src;
#endif
}
std::string ToANSI(const std::string& src)
{
    return ToANSI(src.c_str());
}


template<> std::string to_string(const int& v)
{
    char buf[32];
    sprintf(buf, "%d", v);
    return buf;
}
template<> std::string to_string(const bool& v)
{
    return to_string((int)v);
}
template<> std::string to_string(const float& v)
{
    char buf[32];
    sprintf(buf, "%.3f", v);
    return buf;
}

template<> int from_string(const std::string& v)
{
    return std::atoi(v.c_str());
}
template<> bool from_string(const std::string& v)
{
    return from_string<int>(v) != 0;
}
template<> float from_string(const std::string& v)
{
    return (float)std::atof(v.c_str());
}


template<> picojson::value to_json(const TalkParams& v)
{
    using namespace picojson;
    object t;
    if (v.flags.mute) t["mute"] = value((float)v.mute);
    if (v.flags.volume) t["volume"] = value(v.volume);
    if (v.flags.speed) t["speed"] = value(v.speed);
    if (v.flags.pitch) t["pitch"] = value(v.pitch);
    if (v.flags.intonation) t["intonation"] = value(v.intonation);
    if (v.flags.joy) t["joy"] = value(v.joy);
    if (v.flags.anger) t["anger"] = value(v.anger);
    if (v.flags.sorrow) t["sorrow"] = value(v.sorrow);
    if (v.flags.avator) t["avator"] = value((float)v.avator);
    return value(std::move(t));
}
template<> bool from_json(TalkParams& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<object>())
        return false;
    for (auto& kvp : v.get<object>()) {
        if (kvp.first == "mute") { dst.setMute((int)kvp.second.get<float>() != 0); }
        else if (kvp.first == "volume") { dst.setVolume(kvp.second.get<float>()); }
        else if (kvp.first == "speed") { dst.setSpeed(kvp.second.get<float>()); }
        else if (kvp.first == "pitch") { dst.setPitch(kvp.second.get<float>()); }
        else if (kvp.first == "intonation") { dst.setIntonation(kvp.second.get<float>()); }
        else if (kvp.first == "joy") { dst.setJoy(kvp.second.get<float>()); }
        else if (kvp.first == "anger") { dst.setAnger(kvp.second.get<float>()); }
        else if (kvp.first == "sorrow") { dst.setSorrow(kvp.second.get<float>()); }
        else if (kvp.first == "avator") { dst.setAvator((int)kvp.second.get<float>()); }
    }
    return true;
}

template<> picojson::value to_json(const std::vector<AvatorInfoImpl>& v)
{
    using namespace picojson;
    array t(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        object o;
        o["id"] = value((float)v[i].id);
        o["name"] = value(v[i].name);
        t[i] = value(std::move(o));
    }
    return value(std::move(t));
}
template<> bool from_json(std::vector<AvatorInfoImpl>& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<array>())
        return false;

    for (auto& e : v.get<array>()) {
        if (!e.is<object>())
            continue;
        auto& o = e.get<object>();
        AvatorInfoImpl ai;
        {
            auto it = o.find("id");
            if (it != o.end() && it->second.is<float>())
                ai.id = (int)it->second.get<float>();
        }
        {
            auto it = o.find("name");
            if (it != o.end() && it->second.is<std::string>())
                ai.name = it->second.get<std::string>();
        }
        dst.push_back(ai);
    }
    return true;
}

} // namespace rt

