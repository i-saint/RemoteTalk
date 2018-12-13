#pragma once
#include <vector>
#include <iostream>
#include "rtRawVector.h"

namespace rt {

static constexpr float PI = 3.14159265358979323846264338327950288419716939937510f;
static constexpr float Deg2Rad = PI / 180.0f;
static constexpr float Rad2Deg = 1.0f / (PI / 180.0f);

enum class AudioFormat
{
    Unknown = 0,
    U8,
    S16,
    S24,
    S32,
    F32,
    RawFile = 100,
};
int SizeOf(AudioFormat f);
int GetNumBits(AudioFormat f);

enum class BitrateMode
{
    CBR,
    VBR,
};

struct OggSettings
{
    BitrateMode bitrate_mode = BitrateMode::VBR;
    int target_bitrate = 128 * 1000;
};


class AudioData
{
public:
    AudioFormat format = AudioFormat::Unknown;
    int frequency = 0;
    int channels = 0;
    RawVector<char> data;

public:
    static std::shared_ptr<AudioData> create(std::istream& is);

    AudioData();
    ~AudioData();
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t hash() const;

    void clear();
    template<class T> T* get() { return (T*)data.data(); }
    template<class T> const T* get() const { return (const T*)data.data(); }

    void* allocateByte(size_t num);
    // allocate num_samples * size_of_format bytes
    void* allocateSample(size_t num_samples);
    size_t getSampleLength() const;
    double getDuration() const;

    void convertToMono();
    void increaseChannels(int n); // must be mono before call
    double resample(AudioData& dst, int frequency, int length, double pos = 0.0) const;

    int toFloat(float *dst, int pos = 0, int len = -1, bool multiply = false);
    double resampleFloat(float *dst, int frequency, int channels, int length, double pos = 0.0);

    AudioData& operator+=(const AudioData& v);

    bool exportWave(const std::wstring& path) const;
    bool exportOgg(const std::wstring& path, const OggSettings& settings) const;
};
using AudioDataPtr = std::shared_ptr<AudioData>;

} // namespace rt
