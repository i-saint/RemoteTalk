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
    m_avators.clear();
}

const std::vector<AvatorInfoImpl>& TalkClient::getAvatorList()
{
    if (m_task_avators.valid()) {
        m_task_avators.wait();
        m_task_avators = {};
    }
    return m_avators;
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

bool TalkClient::updateAvatorList()
{
    bool ret = false;
    m_task_avators = std::async(std::launch::async, [this]() {
        try {
            URI uri;
            uri.setPath("/avators");

            HTTPClientSession session{ m_settings.server, m_settings.port };
            session.setTimeout(m_settings.timeout_ms * 1000);

            HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
            session.sendRequest(request);

            HTTPResponse response;
            auto& rs = session.receiveResponse(response);
            if (response.getStatus() == HTTPResponse::HTTP_OK) {
                m_avators.clear();
                std::string line;
                int id;
                char name[256];
                while (std::getline(rs, line)) {
                    if (sscanf(line.c_str(), "%d: %s", &id, name) == 2) {
                        m_avators.push_back({ id, name });
                    }
                }
            }
        }
        catch (Poco::Exception&) {
        }
    });
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

bool TalkClient::talk(const TalkParams& params, const std::string& text, const std::function<void(const AudioData&)>& cb)
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/talk");

#define AddParam(N) if(params.flags.N) { uri.addQueryParameter(#N, to_string(params.##N)); }
        rtEachTalkParams(AddParam)
#undef AddParam
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
