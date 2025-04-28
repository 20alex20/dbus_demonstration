#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <variant>
#include <string>
#include "nlohmann/json.hpp"

typedef std::map<std::string, std::variant<uint32_t, std::string>> dict;

void init(uint32_t& timeout, std::string& timeoutPhrase) {
    std::ifstream initialConfig("~/com.system.configurationManager/confManagerApplication1.json", std::ifstream::binary);
    bool errors = !initialConfig;

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

void establishСonnection(uint32_t& timeout, std::string& timeoutPhrase) {
    sdbus::ServiceName serviceName{"com.system.configurationManager"};
    sdbus::ObjectPath objectPath{"/com/system/configurationManager/Application/confManagerApplication1"};
    auto proxy = sdbus::createProxy(std::move(serviceName), std::move(objectPath));

    sdbus::InterfaceName interfaceName{"com.system.configurationManager.Application.Configuration"};
    proxy->uponSignal("configurationChanged").onInterface(interfaceName).call(
        [&timeout, &timeoutPhrase](const dict& config) {
            timeout = std::get<uint32_t>(config.at("Timeout"));
            timeoutPhrase = std::get<std::string>(config.at("TimeoutPhrase"));
        }
    );
}

int main(int argc, char *argv[]) {
    uint32_t timeout;
    std::string timeoutPhrase;

    init(timeout, timeoutPhrase);
    
    bool connected = false;
    while (true) {
        if (!connected) {
            try {
                establishСonnection(timeout, timeoutPhrase);
                connected = true;
            }
            catch (const sdbus::Error& e) { }
        }
        std::cout << timeoutPhrase << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }

    return 0;
}

