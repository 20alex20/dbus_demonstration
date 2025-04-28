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

void onConfigurationChanged(uint32_t& timeout, std::string& timeoutPhrase, const dict& config) {
    dict::const_iterator search;
    if ((search=config.find("Timeout")) != config.end())
        timeout = std::get<uint32_t>(search->second);
    if ((search= config.find("TimeoutPhrase")) != config.end())
        timeoutPhrase = std::get<std::string>(search->second);
}

void init(uint32_t& timeout, std::string& timeoutPhrase) {
    std::ifstream initial_config("~/com.system.configurationManager/confManagerApplication1.json",
                                 std::ifstream::binary);
    if (!initial_config) {
        timeout = 500;
        timeoutPhrase = "Hello!";
        return;
    }

    nlohmann::json jsonObj;
    initial_config >> jsonObj;
    timeout = jsonObj["Timeout"];
    timeoutPhrase = jsonObj["TimeoutPhrase"];
}

int main(int argc, char *argv[]) {
    uint32_t timeout;
    std::string timeoutPhrase;

    init(timeout, timeoutPhrase);
    
    bool connected = false;
    while (true) {
        if (!connected) try {
            sdbus::ServiceName serviceName{"com.system.configurationManager"};
            sdbus::ObjectPath objectPath{"/com/system/configurationManager/Application/confManagerApplication1"};
            auto proxy = sdbus::createProxy(std::move(serviceName), std::move(objectPath));
            
            sdbus::InterfaceName interfaceName{"com.system.configurationManager.Application.Configuration"};
            proxy->uponSignal("configurationChanged").onInterface(interfaceName).call(
                [&timeout, &timeoutPhrase](const dict& config) {
                    onConfigurationChanged(timeout, timeoutPhrase, config);
                }
            );
            
            connected = true;
        } catch (const sdbus::Error& e) { }
        
        std::cout << timeoutPhrase << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
    }

    return 0;
}

