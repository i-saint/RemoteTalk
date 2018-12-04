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

std::future<AudioDataPtr> TalkClient::send()
{
    try {
        URI uri(m_settings.server);
        for (auto& kvp : m_params)
            uri.addQueryParameter(kvp.first, kvp.second);
        if(!m_text.empty())
            uri.addQueryParameter("t", m_text);

        HTTPClientSession session{ uri.getHost(), uri.getPort() };
        session.setTimeout(m_settings.timeout_ms * 1000);

        HTTPRequest request{ HTTPRequest::HTTP_GET, uri.getPathAndQuery() };
        session.sendRequest(request);

        HTTPResponse response;
        auto& rs = session.receiveResponse(response);
        //std::ostringstream ostr;
        //StreamCopier::copyStream(rs, ostr);
        auto succeeded = response.getStatus() == HTTPResponse::HTTP_OK;
    }
    catch (...) {
    }

    // todo
    return std::future<AudioDataPtr>();
}

} // namespace rt
