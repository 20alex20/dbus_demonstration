#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <pwd.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

typedef std::map<std::string, sdbus::Variant> dict;

void init(uint32_t& timeout, std::string& timeoutPhrase) {
    bool errors = !getenv("HOME");
    std::ifstream initialConfig;
    if (!errors) {
        initialConfig.open(std::string(getpwuid(getuid())->pw_dir) +
                           "/com.system.configurationManager/confManagerApplication1.json", std::ios::binary);
        errors = !initialConfig;
    }

    nlohmann::json jsonObj;
    if (!errors) {
        try {
            initialConfig >> jsonObj;
            errors = !jsonObj.contains("Timeout") || !jsonObj.contains("TimeoutPhrase") ||
                     !jsonObj["Timeout"].is_number_unsigned() || !jsonObj["TimeoutPhrase"].is_string();
        }
        catch (nlohmann::json::parse_error& ex) {
            errors = true;
        }
    }

    if (errors) {
        timeout = 500;
        timeoutPhrase = "Hello!";
    }
    else {
        timeout = jsonObj["Timeout"];
        timeoutPhrase = jsonObj["TimeoutPhrase"];
    }
}

void establishСonnection(std::unique_ptr<sdbus::IProxy>& proxy, uint32_t& timeout, std::string& timeoutPhrase) {
    sdbus::ServiceName serviceName{"com.system.configurationManager"};
    sdbus::ObjectPath objectPath{"/com/system/configurationManager/Application/confManagerApplication1"};
    proxy = sdbus::createProxy(std::move(serviceName), std::move(objectPath));

    sdbus::InterfaceName interfaceName{"com.system.configurationManager.Application.Configuration"};
    proxy->uponSignal("configurationChanged").onInterface(interfaceName).call(
        [&timeout, &timeoutPhrase](const dict& config) {
            dict::const_iterator search;
            if ((search=config.find("Timeout")) != config.end() && search->second.containsValueOfType<uint32_t>())
                timeout = search->second.get<uint32_t>();
            if ((search=config.find("TimeoutPhrase")) != config.end() && search->second.containsValueOfType<std::string>())
                timeoutPhrase = search->second.get<std::string>();
        }
    );
}

int main(int argc, char *argv[]) {
    uint32_t timeout;
    std::string timeoutPhrase;
    init(timeout, timeoutPhrase);

    std::unique_ptr<sdbus::IProxy> proxy;
    bool connected = false;
    while (true) {
        if (!connected) {
            try {
                establishСonnection(proxy, timeout, timeoutPhrase);
                connected = true;
            }
            catch (const sdbus::Error& e) { }
        }
        std::cout << timeoutPhrase << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }

    return 0;
}

