#include "pch.h"
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
    m_params.clear();
    m_text.clear();
}

void TalkClient::setParam(const std::string& name, const std::string& value)
{
    m_params[name] = value;
}

void TalkClient::setText(const std::string& text)
{
    m_text = text;
}

bool TalkClient::send(const std::function<void(const AudioData&)>& cb)
{
    bool ret = false;
    try {
        URI uri;
        uri.setPath("/talk");
        for (auto& kvp : m_params)
            uri.addQueryParameter(kvp.first, kvp.second);
        if (!m_text.empty())
            uri.addQueryParameter("t", m_text);

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
    catch (Poco::Exception& e) {
        auto mes = e.what();
        printf(mes);
    }
    return ret;
}

} // namespace rt
