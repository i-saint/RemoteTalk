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
    client.setText("あー、あー、まいくてすと");

    int n = 0;
    client.send([&](const rt::AudioData& ad) {
        char filename[128];
        sprintf(filename, "%04d.wav", n++);
        ad.exportAsWave(filename);
    });
}
