#pragma once

namespace rtvr2 {

class TalkInterface : public ITalkInterface
{
public:
    rtDefSingleton(TalkInterface);

    TalkInterface();
    ~TalkInterface() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumCasts() const override;
    const rt::CastInfo* getCastInfo(int i) const override;
    bool setText(const char *text) override;

    bool isReady() const override;
    bool isPlaying() const override;
    bool play() override;
    bool stop() override;

    bool setCast(int v) override;
    bool isMainWindowVisible() override;
    bool prepareUI() override;
    void onPlay() override;
    void onStop() override;

#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;
    std::atomic_bool m_is_playing{ false };
};

} // namespace rtvr2
