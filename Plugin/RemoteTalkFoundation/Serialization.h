#pragma once
#include <vector>
#include <string>
#include <memory>
#include "RawVector.h"

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

} // namespace rt
