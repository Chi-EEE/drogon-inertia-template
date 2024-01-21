#include <filesystem>

#include <drogon/drogon.h>

#include "Inertia.hpp"

using namespace drogon;

int wmain(int argc, wchar_t** argv)
{
    std::filesystem::path exe_dir = std::filesystem::weakly_canonical(std::filesystem::path(argv[0])).parent_path();
  	std::string config_json_path = exe_dir.string() + "/settings/config.json";
  	drogon::app().loadConfigFile(config_json_path);
    // `registerHandler()` adds a handler to the desired path. The handler is
    // responsible for generating a HTTP response upon an HTTP request being
    // sent to Drogon
    app().registerHandler(
        "/",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback) {
            std::unordered_map<std::string, Inertia::StringOrLazy> props;
            props.insert({"name", []() { return "[Your Name Here]"; }});
            callback(Inertia::newInertiaResponse(req, props, "App", "/"));
        },
        {Get});

    // Ask Drogon to listen on 127.0.0.1 port 8848. Drogon supports listening
    // on multiple IP addresses by adding multiple listeners. For example, if
    // you want the server also listen on 127.0.0.1 port 5555. Just add another
    // line of addListener("127.0.0.1", 5555)
    LOG_INFO << "Server running on 127.0.0.1:8848";
    app().addListener("127.0.0.1", 8848).run();
}