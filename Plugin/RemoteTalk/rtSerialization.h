#pragma once
#include <vector>
#include <string>
#include <memory>
#include "rtRawVector.h"

namespace rt {

template<class T> struct serializable { static const bool result = false; };

template<class T, bool hs = serializable<T>::result> struct write_impl2;
template<class T> struct write_impl2<T, true> { void operator()(std::ostream& os, const T& v) { v.serialize(os); } };
template<class T> struct write_impl2<T, false> { void operator()(std::ostream& os, const T& v) { os.write((const char*)&v, sizeof(T)); } };

template<class T, bool hs = serializable<T>::result> struct read_impl2;
template<class T> struct read_impl2<T, true> { void operator()(std::istream& is, T& v) { v.deserialize(is); } };
template<class T> struct read_impl2<T, false> { void operator()(std::istream& is, T& v) { is.read((char*)&v, sizeof(T)); } };


template<class T>
struct write_impl
{
    void operator()(std::ostream& os, const T& v)
    {
        write_impl2<T>()(os, v);
    }
};
template<class T>
struct read_impl
{
    void operator()(std::istream& is, T& v)
    {
        read_impl2<T>()(is, v);
    }
};



template<class T>
struct write_impl<RawVector<T>>
{
    void operator()(std::ostream& os, const RawVector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct write_impl<std::string>
{
    void operator()(std::ostream& os, const std::string& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        os.write((const char*)v.data(), size);
    }
};
template<class T>
struct write_impl<std::vector<T>>
{
    void operator()(std::ostream& os, const std::vector<T>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            write_impl<T>()(os, e);
        }
    }
};
template<class T>
struct write_impl<std::shared_ptr<T>>
{
    void operator()(std::ostream& os, const std::shared_ptr<T>& v)
    {
        v->serialize(os);
    }
};
template<class T>
struct write_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::ostream& os, const std::vector<std::shared_ptr<T>>& v)
    {
        auto size = (uint32_t)v.size();
        os.write((const char*)&size, 4);
        for (const auto& e : v) {
            e->serialize(os);
        }
    }
};
template<class T> inline void write(std::ostream& os, const T& v) { return write_impl<T>()(os, v); }



template<class T>
struct read_impl<RawVector<T>>
{
    void operator()(std::istream& is, RawVector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize_discard(size);
        is.read((char*)v.data(), sizeof(T) * size);
    }
};
template<>
struct read_impl<std::string>
{
    void operator()(std::istream& is, std::string& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        is.read((char*)v.data(), size);
    }
};
template<class T>
struct read_impl<std::vector<T>>
{
    void operator()(std::istream& is, std::vector<T>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<T>()(is, e);
        }
    }
};
template<class T>
struct read_impl<std::shared_ptr<T>>
{
    void operator()(std::istream& is, std::shared_ptr<T>& v)
    {
        v = T::create(is);
    }
};
template<class T>
struct read_impl<std::vector<std::shared_ptr<T>>>
{
    void operator()(std::istream& is, std::vector<std::shared_ptr<T>>& v)
    {
        uint32_t size = 0;
        is.read((char*)&size, 4);
        v.resize(size);
        for (auto& e : v) {
            read_impl<std::shared_ptr<T>>()(is, e);
        }
    }
};
template<class T> inline void read(std::istream& is, T& v) { return read_impl<T>()(is, v); }


template<class T>
struct hash_impl;

template<class T>
struct hash_impl<RawVector<T>>
{
    uint64_t operator()(const RawVector<T>& v)
    {
        uint64_t ret = 0;
        if (sizeof(T) * v.size() >= 8)
            ret = *(const uint64_t*)((const char*)v.end() - 8);
        return ret;
    }
};

template<> struct hash_impl<bool> { uint64_t operator()(bool v) { return (uint32_t)v; } };
template<> struct hash_impl<int> { uint64_t operator()(int v) { return (uint32_t&)v; } };
template<> struct hash_impl<float> { uint64_t operator()(float v) { return (uint32_t&)v; } };

template<class T> inline uint64_t gen_hash(const T& v) { return hash_impl<T>()(v); }



template<class T> inline std::string to_string(const T& v);

template<> inline std::string to_string(const int& v)
{
    char buf[32];
    sprintf(buf, "%d", v);
    return buf;
}
template<> inline std::string to_string(const bool& v)
{
    return to_string((int)v);
}
template<> inline std::string to_string(const float& v)
{
    char buf[32];
    sprintf(buf, "%.3f", v);
    return buf;
}


template<class T> inline T from_string(const std::string& v);

template<> inline int from_string(const std::string& v)
{
    return std::atoi(v.c_str());
}
template<> inline bool from_string(const std::string& v)
{
    return from_string<int>(v) != 0;
}
template<> inline float from_string(const std::string& v)
{
    return (float)std::atof(v.c_str());
}

} // namespace rt
