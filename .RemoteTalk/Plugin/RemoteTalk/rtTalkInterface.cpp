#include "pch.h"
#include "rtTalkInterface.h"

namespace rt {

TalkParams::Proxy::operator float() const
{
    return self->params[index];
}

TalkParams::Proxy& TalkParams::Proxy::operator=(float v)
{
    self->param_flags = self->param_flags | (1 << index);
    self->params[index] = v;
    return *this;
}


TalkParams::Proxy TalkParams::operator[](int i)
{
    return { this, i };
}

const TalkParams::Proxy TalkParams::operator[](int i) const
{
    return { (TalkParams*)this, i };
}

bool TalkParams::isSet(int i) const
{
    return (param_flags & (1 << i)) != 0;
}

uint32_t TalkParams::hash() const
{
    uint32_t ret = 0;
    for (int i = 0; i < MaxParams; ++i) {
        if (isSet(i)) {
            auto u = (uint32_t)(params[i] * 100.0f);
            int s = (i * 9) % 32;
            ret ^= (u << s) | (u >> (32 - s));
        }
    }
    return ret;
}

} // namespace rt
