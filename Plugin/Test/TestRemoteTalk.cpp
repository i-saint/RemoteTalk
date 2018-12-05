#include "pch.h"
#include "Test.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"


static rt::TalkClientSettings GetClientSettings()
{
    rt::TalkClientSettings ret;
    GetArg("server", ret.server);
    int port;
    if (GetArg("port", port))
        ret.port = (uint16_t)port;
    return ret;
}

TestCase(RemoteTalkClient)
{
    rt::TalkClient client(GetClientSettings());
    client.setText("hello voiceroid!");

    int n = 0;
    rt::AudioData sequence;
    client.send([&](const rt::AudioData& ad) {
        if (ad.data.empty())
            return;
        sequence += ad;
    });
    sequence.exportAsWave("hello_voiceroid.wav");
}
