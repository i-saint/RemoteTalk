#include "pch.h"
#include "rtFoundation.h"
#include "rtTalkServer.h"

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
                auto mes = std::make_shared<TalkServer::TalkMessage>();
                Poco::URI::decode(&uri[pos + 2], mes->text, true);
                mes->text = ToANSI(mes->text.c_str());

                response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
                response.setContentType("application/octet-stream");
                mes->respond_stream = &response.send();

                m_server->addMessage(mes);
                if (mes->wait()) {
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
    for (int i = 0; i < 10000; ++i) {
        if (ready.load())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    if (task.valid()) {
        task.wait();
    }
    return ready.load();
}

bool TalkServer::Message::isSending()
{
    return task.valid() && task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}


TalkServer::TalkServer()
{
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
        if (mes->ready.load()) {
            if (mes->isSending()) {
                break;
            }
            else {
                mes.reset();
                continue;
            }
        }

        if (auto *pmes = dynamic_cast<ParamMessage*>(mes.get())) {
            for (auto& nvp : pmes->params)
                onSetParam(nvp.first, nvp.second);
        }
        if (auto *tmes = dynamic_cast<TalkMessage*>(mes.get())) {
            tmes->task = onTalk(tmes->text, *tmes->respond_stream);
        }

        mes->ready = true;
        if (mes->isSending())
            break;
        else
            mes.reset();
    }
    m_messages.erase(
        std::remove_if(m_messages.begin(), m_messages.end(), [](MessagePtr &p) { return !p; }),
        m_messages.end());
}

void TalkServer::addMessage(MessagePtr mes)
{
    lock_t lock(m_mutex);
    m_messages.push_back(mes);
}

} // namespace rt
