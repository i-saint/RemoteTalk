#include "pch.h"
#include "rtTalkReceiver.h"

namespace rt {

using namespace Poco::Net;
using Poco::URI;

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

TalkReceiver::TalkReceiver()
{
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

} // namespace rt
