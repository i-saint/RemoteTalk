#include "pch.h"
#include "rtFoundation.h"
#include "rtServer.h"

namespace rt {

using namespace Poco::Net;
using Poco::URI;

std::string ToANSI(const char *src)
{
#ifdef _WIN32
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize);
    ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to ANSI
    const int u8size = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size);
    ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
#else
    return src;
#endif
}

void ServeText(Poco::Net::HTTPServerResponse& response, const std::string& data, int stat, const std::string& mimetype)
{
    response.setStatus((HTTPResponse::HTTPStatus)stat);
    response.setContentType(mimetype);
    response.setContentLength(data.size());

    auto& os = response.send();
    os.write(data.data(), data.size());
    os.flush();
}

void ServeBinary(Poco::Net::HTTPServerResponse& response, RawVector<char>& data, const std::string& mimetype)
{
    response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
    response.setContentType(mimetype);
    response.setContentLength(data.size());

    auto& os = response.send();
    os.write(data.data(), data.size());
    os.flush();
}

class TalkServerRequestHandler : public HTTPRequestHandler
{
public:
    TalkServerRequestHandler(TalkServer *server);
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

private:
    TalkServer *m_server = nullptr;
};

class TalkServerRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    TalkServerRequestHandlerFactory(TalkServer *server);
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;

private:
    TalkServer *m_server = nullptr;
};

TalkServerRequestHandler::TalkServerRequestHandler(TalkServer *server)
    : m_server(server)
{
}

void TalkServerRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto& uri = request.getURI();

    bool handled = false;
    if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
        if (strncmp(uri.c_str(), "/talk", 5) == 0) {
            auto pos = uri.find("t=");
            if (pos != std::string::npos) {
                auto *mes = new TalkServer::TalkMessage();
                Poco::URI::decode(&uri[pos + 2], mes->text, true);
                mes->text = ToANSI(mes->text.c_str());

                TalkServer::MessagePtr pmes(mes);
                m_server->addMessage(pmes);
                if (mes->wait() && mes->data) {
                    // todo
                    ServeText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
                    handled = true;
                }
            }
        }
        else if (strncmp(uri.c_str(), "/param", 6) == 0) {
            // todo
            ServeText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
            handled = true;
        }
    }

    if (!handled)
        ServeText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
}

TalkServerRequestHandlerFactory::TalkServerRequestHandlerFactory(TalkServer *server)
    : m_server(server)
{
}

HTTPRequestHandler* TalkServerRequestHandlerFactory::createRequestHandler(const HTTPServerRequest&)
{
    return new TalkServerRequestHandler(m_server);
}


bool TalkServer::Message::wait()
{
    for (int i = 0; i < 1000; ++i) {
        if (ready.load())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    return ready.load();
}

TalkServer::~TalkServer()
{
    stop();
}

void TalkServer::setSettings(const TalkServerSettings& v)
{
    m_settings = v;
}

bool TalkServer::start()
{
    if (!m_server) {
        auto* params = new HTTPServerParams;
        if (m_settings.max_queue > 0)
            params->setMaxQueued(m_settings.max_queue);
        if (m_settings.max_threads > 0)
            params->setMaxThreads(m_settings.max_threads);

        try {
            ServerSocket svs(m_settings.port);
            m_server.reset(new HTTPServer(new TalkServerRequestHandlerFactory(this), svs, params));
            m_server->start();
        }
        catch (Poco::IOException &e) {
            printf("%s\n", e.what());
            return false;
        }
    }

    return true;
}

void TalkServer::stop()
{
    m_server.reset();
}

void TalkServer::processMessages()
{
    lock_t lock(m_mutex);

    for (auto& mes : m_messages) {
        if (auto *pmes = dynamic_cast<ParamMessage*>(mes.get())) {
            for (auto& nvp : pmes->params)
                onSetParam(nvp.first, nvp.second);
        }
        if (auto *tmes = dynamic_cast<TalkMessage*>(mes.get())) {
            onTalk(tmes->text);
        }
        mes->ready = true;
    }
    m_messages.clear();
}

void TalkServer::addMessage(MessagePtr mes)
{
    lock_t lock(m_mutex);
    m_messages.push_back(mes);
}



class TalkReceiverRequestHandler : public HTTPRequestHandler
{
public:
    TalkReceiverRequestHandler(TalkReceiver *server);
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

private:
    TalkReceiver *m_server = nullptr;
};

class TalkReceiverRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    TalkReceiverRequestHandlerFactory(TalkReceiver *server);
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;

private:
    TalkReceiver *m_server = nullptr;
};

TalkReceiverRequestHandler::TalkReceiverRequestHandler(TalkReceiver *server)
    : m_server(server)
{
}

void TalkReceiverRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto& uri = request.getURI();

    bool handled = false;
    if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
    }

    if (!handled)
        ServeText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
}

TalkReceiverRequestHandlerFactory::TalkReceiverRequestHandlerFactory(TalkReceiver *server)
    : m_server(server)
{
}

HTTPRequestHandler* TalkReceiverRequestHandlerFactory::createRequestHandler(const HTTPServerRequest&)
{
    return new TalkReceiverRequestHandler(m_server);
}

TalkReceiver::~TalkReceiver()
{
    stop();
}

void TalkReceiver::setSettings(const TalkServerSettings & v)
{
}

bool TalkReceiver::start()
{
    if (!m_server) {
        auto* params = new HTTPServerParams;
        if (m_settings.max_queue > 0)
            params->setMaxQueued(m_settings.max_queue);
        if (m_settings.max_threads > 0)
            params->setMaxThreads(m_settings.max_threads);

        try {
            ServerSocket svs(m_settings.port);
            m_server.reset(new HTTPServer(new TalkReceiverRequestHandlerFactory(this), svs, params));
            m_server->start();
        }
        catch (Poco::IOException &e) {
            printf("%s\n", e.what());
            return false;
        }
    }

    return true;
}

void TalkReceiver::stop()
{
    m_server.reset();
}




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
