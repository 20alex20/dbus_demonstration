#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <variant>
#include <string>
#include "nlohmann/json.hpp"

#define DBUS_ERROR(str) throw sdbus::Error(sdbus::Error::Name{"com.system.configurationManager.Error"}, str);

namespace fs = std::filesystem;

typedef std::map<std::string, std::variant<uint32_t, std::string>> dict;
typedef struct {
    std::unique_ptr<sdbus::IObject> object;
    std::string name;
    dict config;
} application;

dict getConfig(std::string path) {
    std::ifstream initialConfig(path, std::ifstream::binary);
    if (!initialConfig)
        DBUS_ERROR("Unable to read configuration file");

    nlohmann::json jsonObj;
    try {
        initialConfig >> jsonObj;
    }
    catch (nlohmann::json::parse_error& ex) {
        DBUS_ERROR("Error parsing config file");
    }

    if (!jsonObj.contains("Timeout") || !jsonObj.contains("TimeoutPhrase"))
        DBUS_ERROR("Not all parameters are listed in the configuration file");
    if (!jsonObj["Timeout"].is_number_unsigned() || jsonObj["TimeoutPhrase"].is_string())
        DBUS_ERROR("The types of parameter values do not match the required ones");
    return { { "Timeout", static_cast<uint32_t>(jsonObj["Timeout"]) },
             { "TimeoutPhrase", static_cast<std::string>(jsonObj["TimeoutPhrase"]) } };
}

void generateObject(sdbus::IConnection* connection, const std::string& applicationName) {
    sdbus::ObjectPath objectPath{"/com/system/configurationManager/Application/" + applicationName};
    auto app = new application{ sdbus::createObject(*connection, std::move(objectPath)), applicationName, { } };

    auto getConfiguration = [app]() {
        if (app->config.empty())
            app->config = getConfig("~/com.system.configurationManager/" + app->name + ".json");
        return app->config;
    };

    auto changeConfiguration = [app](const std::string& key, const std::variant<uint32_t, std::string>& value) {
        if (key != "Timeout" && key != "TimeoutPhrase")
            DBUS_ERROR("Unknown key transmitted");
        if (app->config.empty())
            app->config = getConfig("~/com.system.configurationManager/" + app->name + ".json");
        app->config[key] = value;
        app->object->emitSignal("configurationChanged")
                    .onInterface("com.system.configurationManager.Application.Configuration")
                    .withArguments(app->config);
    };

    app->object->addVTable(sdbus::registerMethod("ChangeConfiguration").implementedAs(std::move(changeConfiguration)),
                           sdbus::registerMethod("GetConfiguration").implementedAs(std::move(getConfiguration)),
                           sdbus::registerSignal("configurationChanged").withParameters<dict>())
                .forInterface("com.system.configurationManager.Application.Configuration");
}

int main(int argc, char *argv[]) {
    if (!fs::exists("~/com.system.configurationManager") || !fs::is_directory("~/com.system.configurationManager")) {
        std::cerr << "Directory \"~/com.system.configurationManager\" does not exist" << std::endl;
        return 1;
    }

    sdbus::ServiceName serviceName{"com.system.configurationManager"};
    auto connection = sdbus::createBusConnection(serviceName);

    uint32_t configFilesNumber = 0, errorsDbusObjectNumber = 0;
    for (const auto& entry : fs::directory_iterator("~/com.system.configurationManager")) {
        std::string path = entry.path();
        if (fs::is_directory(path))
            continue;
        size_t indexSlashPlus = path.rfind('/') + 1, indexDot = path.rfind(".json");
        if (indexDot == std::string::npos || indexDot != path.size()-5 || indexDot == indexSlashPlus)
            continue;
        configFilesNumber += 1;

        std::string applicationName = path.substr(indexSlashPlus, indexDot - indexSlashPlus);
        try {
            generateObject(connection.get(), applicationName);
        }
        catch (const sdbus::Error& e) {
             std::cerr << "Error creating dbus-object for application \"" << applicationName << "\"" << std::endl;
             errorsDbusObjectNumber += 1;
        }
    }

    if (!configFilesNumber)
        std::cout << "Configuration files not found" << std::endl;
    else if (configFilesNumber - errorsDbusObjectNumber) {
        connection->enterEventLoop();
        std::cout << "Service launched successfully" << std::endl;
    }
    return 0;
}
