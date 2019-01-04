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

bool TalkClient::isServerAvailable()
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/ready");

        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(100 * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        session.receiveResponse(response);
        ret = response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (Poco::Exception&) {
    }
    return ret;
}

bool TalkClient::stats(TalkServerStats& stats)
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/stats");

        HTTPClientSession session{ m_settings.server, m_settings.port };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        if (response.getStatus() == HTTPResponse::HTTP_OK) {
            std::string s(std::istreambuf_iterator<char>(rs), {});
            TalkServer::StatsMessage mes;
            if (mes.from_json(s)) {
                stats = std::move(mes.stats);
                ret = true;
            }
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

bool TalkClient::play(const TalkParams& params, const std::string& text, const std::function<void(const AudioData&)>& cb)
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/talk");

        uri.addQueryParameter("mute", to_string(params.mute));
        uri.addQueryParameter("force_mono", to_string(params.force_mono));
        uri.addQueryParameter("cast", to_string(params.cast));
        for (int i = 0; i < TalkParams::MaxParams; ++i) {
            if (params.isSet(i)) {
                char name[128];
                sprintf(name, "a%d", i);
                uri.addQueryParameter(name, to_string((float)params[i]));
            }
        }
        if (!text.empty())
            uri.addQueryParameter("text", text);

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

} // namespace rt
