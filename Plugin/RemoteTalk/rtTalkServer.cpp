#include "pch.h"
#include "rtFoundation.h"
#include "rtSerialization.h"
#include "rtTalkServer.h"
#include "rtTalkClient.h"
#include "picojson/picojson.h"

namespace rt {

using namespace Poco::Net;
using Poco::URI;

bool SaveServerSettings(const TalkServerSettingsTable& src, const std::string& path)
{
    std::ofstream fo(path);
    if (!fo)
        return false;
    fo << to_string(to_json(src));
    return true;
}

bool LoadServerSettings(TalkServerSettingsTable& dst, const std::string& path)
{
    std::string s;
    {
        std::ifstream fin(path);
        if (!fin)
            return false;
        s = std::string(std::istreambuf_iterator<char>(fin), {});
    }
    picojson::value val;
    picojson::parse(val, s);
    return from_json(dst, val);
}

TalkServerSettings GetOrAddServerSettings(const std::string& path, const std::string& key_, uint16_t default_port)
{
    rt::TalkServerSettingsTable table;
    rt::LoadServerSettings(table, path);

    auto key = key_;
    for (auto& c : key) {
        if (c == '\\')
            c = '/';
    }

    uint16_t port = default_port;
    auto it = table.find(key);
    if (it == table.end()) {
        auto& settings = table[key];
        for (auto& kvp : table)
            port = std::max<uint16_t>(port, kvp.second.port + 1);
        settings.port = (uint16_t)port;
        rt::SaveServerSettings(table, path);
        return settings;
    }
    else {
        return it->second;
    }
}

bool WaitUntilServerRespond(uint16_t port, int timeout_ms)
{
    const int NumTry = 5;
    int interval = timeout_ms / NumTry;

    TalkClientSettings settings;
    settings.port = port;
    TalkClient client(settings);

    for (int i = 0; i < NumTry; ++i) {
        try {
            TalkServerStats stats;
            if (client.stats(stats) && stats.casts.size() > 0)
                break;
        }
        catch (...) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

    }
    return false;
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
    URI uri(request.getURI());

    bool handled = false;
    if (uri.getPath() == "/ready") {
        ServeText(response, m_server->isReady() ? "1" : "0", HTTPResponse::HTTPStatus::HTTP_OK);
    }
    else if (uri.getPath() == "/talk") {
        auto mes = std::make_shared<TalkServer::TalkMessage>();

        auto qparams = uri.getQueryParameters();
        for (auto& nvp : qparams) {
            if (nvp.first == "mute") {
                mes->params.mute = rt::from_string<int>(nvp.second);
            }
            else if (nvp.first == "force_mono") {
                mes->params.force_mono = rt::from_string<int>(nvp.second);
            }
            else if (nvp.first == "cast") {
                mes->params.cast = (short)rt::from_string<int>(nvp.second);
            }
            else if (nvp.first == "text") {
                Poco::URI::decode(nvp.second, mes->text, true);
                mes->text = ToANSI(mes->text.c_str());
            }
            else {
                for (int i = 0; i < TalkParams::MaxParams; ++i) {
                    char name[128];
                    sprintf(name, "a%d", i);
                    if (nvp.first == name) {
                        mes->params[i] = rt::from_string<float>(nvp.second);
                    }
                }
            }
        }

        {
            std::string s(std::istreambuf_iterator<char>(request.stream()), {});
            if (!s.empty())
                mes->from_json(s);
        }

        response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
        response.setContentType("application/octet-stream");
        mes->respond_stream = &response.send();

        m_server->addMessage(mes);
        if (mes->wait())
            handled = true;
    }
    else if (uri.getPath() == "/stop") {
        auto mes = std::make_shared<TalkServer::StopMessage>();
        m_server->addMessage(mes);
        if (mes->wait())
            handled = true;
        ServeText(response, "ok", HTTPResponse::HTTPStatus::HTTP_OK);
    }
    else if (uri.getPath() == "/stats") {
        auto mes = std::make_shared<TalkServer::StatsMessage>();
        m_server->addMessage(mes);
        if (mes->wait())
            handled = true;
        ServeText(response, mes->to_json(), HTTPResponse::HTTPStatus::HTTP_OK, "application/json");
    }
#ifdef rtDebug
    else if (uri.getPath() == "/debug") {
        auto mes = std::make_shared<TalkServer::DebugMessage>();
        auto qparams = uri.getQueryParameters();
        for (auto& pair : qparams)
            mes->params[pair.first] = pair.second;

        m_server->addMessage(mes);
        if (mes->wait())
            handled = true;
        ServeText(response, "ok", HTTPResponse::HTTPStatus::HTTP_OK);
    }
#endif

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
        if (handled.load())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    if (task.valid()) {
        task.wait();
    }
    return handled.load();
}

bool TalkServer::Message::isProcessing()
{
    return task.valid() && task.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
}

std::string TalkServer::StatsMessage::to_json()
{
    using namespace picojson;
    return rt::to_json(stats).serialize(true);
}

bool TalkServer::StatsMessage::from_json(const std::string& str)
{
    using namespace picojson;
    value val;
    parse(val, str);

    bool ret = false;
    if (rt::from_json(stats, val))
        ret = true;
    return ret;
}


std::string TalkServer::TalkMessage::to_json()
{
    using namespace picojson;
    object ret;
    ret["params"] = rt::to_json(params);
    ret["text"] = rt::to_json(text);
    return value(std::move(ret)).serialize(true);
}

bool TalkServer::TalkMessage::from_json(const std::string & str)
{
    using namespace picojson;
    value val;
    parse(val, str);

    bool ret = false;
    if (rt::from_json(params, val.get("params")))
        ret = true;
    if (rt::from_json(text, val.get("text")))
        ret = true;
    return ret;
}


#ifdef rtDebug
std::string TalkServer::DebugMessage::to_json()
{
    using namespace picojson;
    auto val = rt::to_json(params);
    return val.to_str();
}

bool TalkServer::DebugMessage::from_json(const std::string& str)
{
    using namespace picojson;
    value val;
    parse(val, str);
    return rt::from_json(params, val);
}
#endif


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

bool TalkServer::isRunning() const
{
    return m_server != nullptr;
}

void TalkServer::processMessages()
{
    lock_t lock(m_mutex);
    for (auto& mes : m_messages) {
        if (!mes->handled.load()) {
            auto s = Status::Failed;
            if (auto *talk = dynamic_cast<TalkMessage*>(mes.get()))
                s = onTalk(*talk);
            else if (auto *stop = dynamic_cast<StopMessage*>(mes.get()))
                s = onStop(*stop);
            else if (auto *stats = dynamic_cast<StatsMessage*>(mes.get()))
                s = onStats(*stats);
#ifdef rtDebug
            else if (auto *dbg = dynamic_cast<DebugMessage*>(mes.get()))
                s = onDebug(*dbg);
#endif

            if (s == Status::Pending)
                break;
            mes->status = s;
            mes->handled = true;
            mes.reset();
        }
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
