#include "pch.h"
#include "rtSerialization.h"
#include "rtTalkClient.h"

namespace rt {

using namespace Poco::Net;
using Poco::URI;


TalkClient::TalkClient(const TalkClientSettings& settings)
    : m_settings(settings)
{
}

TalkClient::~TalkClient()
{
}

void TalkClient::clear()
{
    m_parmas = {};
    m_text.clear();
}


#define Set(V) m_parmas.flags.fields.V = 1; m_parmas.##V = v;
void TalkClient::setSilence(bool v)     { Set(silence); }
void TalkClient::setVolume(float v)     { Set(volume); }
void TalkClient::setSpeed(float v)      { Set(speed); }
void TalkClient::setPitch(float v)      { Set(pitch); }
void TalkClient::setIntonation(float v) { Set(intonation); }
void TalkClient::setJoy(float v)        { Set(joy); }
void TalkClient::setAnger(float v)      { Set(anger); }
void TalkClient::setSorrow(float v)     { Set(sorrow); }
#undef Set

void TalkClient::setText(const std::string& text)
{
    m_text = text;
}

bool TalkClient::talk(const std::function<void(const AudioData&)>& cb)
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/talk");

#define AddParam(N) if(m_parmas.flags.fields.N) { uri.addQueryParameter(#N, to_string(m_parmas.##N)); }
        rtEachTalkParams(AddParam)
#undef AddParam
        if (!m_text.empty())
            uri.addQueryParameter("text", m_text);

        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        AudioData audio_data;
        for (;;) {
            audio_data.deserialize(rs);
            if(cb)
                cb(audio_data);

            // empty data means end of stream
            if (audio_data.data.empty())
                break;
        }
        ret = response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (Poco::Exception&) {
    }
    return ret;
}

bool TalkClient::stop()
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/stop");

        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        if (response.getStatus() == HTTPResponse::HTTP_OK) {
            char r;
            rs.read(&r, 1);
            ret = r == '1';
        }
    }
    catch (Poco::Exception&) {
    }
    return ret;
}

bool TalkClient::ready()
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/ready");

        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        if (response.getStatus() == HTTPResponse::HTTP_OK) {
            char r;
            rs.read(&r, 1);
            ret = r == '1';
        }
    }
    catch (Poco::Exception&) {
    }
    return ret;
}

} // namespace rt
