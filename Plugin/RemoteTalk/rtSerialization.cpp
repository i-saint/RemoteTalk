#include "pch.h"
#include "rtSerialization.h"
#include "rtTalkInterfaceImpl.h"
#include "rtTalkServer.h"
#include "picojson/picojson.h"

namespace rt {

void Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringA(buf);
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

void Print(const wchar_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    wchar_t buf[1024];
    _vsnwprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringW(buf);
#else
    vwprintf(fmt, args);
#endif
    va_end(args);
}


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

std::string ToMBS(const wchar_t * src)
{
    using converter_t = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;
    return converter_t().to_bytes(src);
}
std::string ToMBS(const std::wstring& src)
{
    return ToMBS(src.c_str());
}

std::wstring ToWCS(const char * src)
{
    using converter_t = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;
    return converter_t().from_bytes(src);
}
std::wstring ToWCS(const std::string & src)
{
    return ToWCS(src.c_str());
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


template<> picojson::value to_json(const bool& v)
{
    using namespace picojson;
    return value(v);
}
template<> bool from_json(bool& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<bool>())
        return false;
    dst = v.get<bool>();
    return true;
}

template<> picojson::value to_json(const int& v)
{
    using namespace picojson;
    return value((float)v);
}
template<> bool from_json(int& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<float>())
        return false;
    dst = (int)v.get<float>();
    return true;
}

template<> picojson::value to_json(const float& v)
{
    using namespace picojson;
    return value(v);
}
template<> bool from_json(float& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<float>())
        return false;
    dst = v.get<float>();
    return true;
}

template<> picojson::value to_json(const std::string& v)
{
    using namespace picojson;
    return value(v);
}
template<> bool from_json(std::string& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<std::string>())
        return false;
    dst = v.get<std::string>();
    return true;
}

template<> picojson::value to_json(const TalkParams& v)
{
    using namespace picojson;
    object t;
    t["mute"] = value((float)v.mute);
    t["force_mono"] = value((float)v.force_mono);
    t["cast"] = value((float)v.cast);
    {
        array params;
        for (int i = 0; i < v.num_params; ++i)
            params.push_back(value(v.params[i]));
        t["params"] = value(params);
    }
    return value(std::move(t));
}
template<> bool from_json(TalkParams& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<object>())
        return false;
    for (auto& kvp : v.get<object>()) {
        if      (kvp.first == "mute") { dst.mute = (int)kvp.second.get<float>(); }
        else if (kvp.first == "force_mono") { dst.force_mono = (int)kvp.second.get<float>(); }
        else if (kvp.first == "cast") { dst.cast = (int)kvp.second.get<float>(); }
        else if (kvp.first == "params") {
            if (kvp.second.is<array>()) {
                array params = kvp.second.get<array>();
                int n = std::min((int)params.size(), TalkParams::max_params);
                dst.num_params = n;
                for (int i = 0; i < n; ++i) {
                    if (params[i].is<float>())
                        dst.params[i] = params[i].get<float>();
                }
            }
        }
    }
    return true;
}

template<> picojson::value to_json(const CastInfoImpl& v)
{
    using namespace picojson;
    object o;
    o["id"] = value((float)v.id);
    o["name"] = value(v.name);

    if (!v.param_names.empty()) {
        array exp;
        for (auto& pname : v.param_names)
            exp.push_back(value(pname));
        o["param_names"] = value(exp);
    }

    return value(std::move(o));
}
template<> bool from_json(CastInfoImpl& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<object>())
        return false;

    auto& o = v.get<object>();
    {
        auto it = o.find("id");
        if (it != o.end() && it->second.is<float>())
            dst.id = (int)it->second.get<float>();
    }
    {
        auto it = o.find("name");
        if (it != o.end() && it->second.is<std::string>())
            dst.name = it->second.get<std::string>();
    }
    {
        auto it = o.find("param_names");
        if (it != o.end() && it->second.is<array>()) {
            auto& exp = it->second.get<array>();
            dst.param_names.clear();
            for (auto& pname : exp)
                dst.param_names.push_back(pname.get<std::string>());
        }
    }
    return true;
}

template<> picojson::value to_json(const CastList& v)
{
    using namespace picojson;
    array t(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        t[i] = to_json(v[i]);
    return value(std::move(t));
}
template<> bool from_json(CastList& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<array>())
        return false;

    for (auto& e : v.get<array>()) {
        CastInfoImpl ai;
        if(from_json(ai, e))
            dst.push_back(ai);
    }
    return true;
}

template<> picojson::value to_json(const TalkServerStats& v)
{
    using namespace picojson;
    object ret;
    ret["host"] = rt::to_json(v.host);
    ret["plugin_version"] = rt::to_json(v.plugin_version);
    ret["protocol_version"] = rt::to_json(v.protocol_version);
    ret["params"] = rt::to_json(v.params);
    ret["casts"] = rt::to_json(v.casts);
    return value(std::move(ret));
}

template<> bool from_json(TalkServerStats& dst, const picojson::value& v)
{
    using namespace picojson;
    bool ret = false;
    if (rt::from_json(dst.host, v.get("host")))
        ret = true;
    if (rt::from_json(dst.plugin_version, v.get("plugin_version")))
        ret = true;
    if (rt::from_json(dst.protocol_version, v.get("protocol_version")))
        ret = true;
    if (rt::from_json(dst.params, v.get("params")))
        ret = true;
    if (rt::from_json(dst.casts, v.get("casts")))
        ret = true;
    return ret;
}


template<> picojson::value to_json(const std::map<std::string, std::string>& v)
{
    using namespace picojson;
    object t;
    for (auto& kvp : v)
        t[kvp.first] = value(kvp.second);
    return value(std::move(t));
}
template<> bool from_json(std::map<std::string, std::string>& dst, const picojson::value& v)
{
    using namespace picojson;
    if (!v.is<object>())
        return false;

    auto& o = v.get<object>();
    for (auto& kvp : o) {
        if (kvp.second.is<std::string>())
            dst[kvp.first] = kvp.second.get<std::string>();
    }
    return true;
}

} // namespace rt

