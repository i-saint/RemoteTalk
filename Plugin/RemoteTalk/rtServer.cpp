#include "pch.h"
#include "rtServer.h"

namespace rt {

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
        if (uri == "/talk") {
            auto pos = uri.find("t=");
            if (pos != std::string::npos) {
                std::string text;
                Poco::URI::decode(&uri[pos + 2], text, true);
                auto data = m_server->onTalk(text).get();
                if (data) {
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
