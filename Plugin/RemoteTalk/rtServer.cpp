#include "pch.h"
#include "rtFoundation.h"
#include "rtServer.h"

namespace rt {

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

using namespace Poco::Net;

class RequestHandler : public HTTPRequestHandler
{
public:
    RequestHandler(Server *server);
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

private:
    Server *m_server = nullptr;
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    RequestHandlerFactory(Server *server);
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &request) override;

private:
    Server *m_server = nullptr;
};

RequestHandler::RequestHandler(Server *server)
    : m_server(server)
{
}

void RequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    auto& uri = request.getURI();

    bool handled = false;
    if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
        if (strncmp(uri.c_str(), "/talk", 5) == 0) {
            auto pos = uri.find("t=");
            if (pos != std::string::npos) {
                auto *mes = new Server::TalkMessage();
                Poco::URI::decode(&uri[pos + 2], mes->text, true);
                mes->text = ToANSI(mes->text.c_str());

                Server::MessagePtr pmes(mes);
                m_server->addMessage(pmes);
                if (mes->wait() && mes->data) {
                    // todo
                    m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
                    handled = true;
                }
            }
        }
        else if (uri == "/param") {
            // todo
            m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
            handled = true;
        }
    }

    if (!handled)
        m_server->serveText(response, "", HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
}

RequestHandlerFactory::RequestHandlerFactory(Server *server)
    : m_server(server)
{
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest&)
{
    return new RequestHandler(m_server);
}


bool Server::Message::wait()
{
    for (int i = 0; i < 1000; ++i) {
        if (ready.load())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    return ready.load();
}

Server::~Server()
{
    stop();
}

void Server::setSettings(const ServerSettings& v)
{
    m_settings = v;
}

bool Server::start()
{
    if (!m_server) {
        auto* params = new HTTPServerParams;
        if (m_settings.max_queue > 0)
            params->setMaxQueued(m_settings.max_queue);
        if (m_settings.max_threads > 0)
            params->setMaxThreads(m_settings.max_threads);

        try {
            ServerSocket svs(m_settings.port);
            m_server.reset(new HTTPServer(new RequestHandlerFactory(this), svs, params));
            m_server->start();
        }
        catch (Poco::IOException &e) {
            printf("%s\n", e.what());
            return false;
        }
    }

    return true;
}

void Server::stop()
{
    m_server.reset();
}

void Server::processMessages()
{
    lock_t lock(m_mutex);

    for (auto& mes : m_messages) {
        if (auto *pmes = dynamic_cast<ParamMessage*>(mes.get())) {
            for (auto& nvp : pmes->params)
                onSetParam(nvp.first, nvp.second);
        }
        else if (auto *tmes = dynamic_cast<TalkMessage*>(mes.get())) {
            onTalk(tmes->text);
        }
        else {
            throw std::runtime_error("should not be here");
        }
        mes->ready = true;
    }
    m_messages.clear();
}

void Server::addMessage(MessagePtr mes)
{
    lock_t lock(m_mutex);
    m_messages.push_back(mes);
}

void Server::serveText(Poco::Net::HTTPServerResponse & response, const char * text, int stat)
{
    size_t size = std::strlen(text);

    response.setStatus((HTTPResponse::HTTPStatus)stat);
    response.setContentType("text/plain");
    response.setContentLength(size);

    auto& os = response.send();
    os.write(text, size);
    os.flush();
}

} // namespace rt
