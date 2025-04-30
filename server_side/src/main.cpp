#include <sdbus-c++/sdbus-c++.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>
#include <string>
#include <pwd.h>
#include <unistd.h>
#include "nlohmann/json.hpp"

#define DBUS_ERROR(str) throw sdbus::Error(sdbus::Error::Name{"com.system.configurationManager.Error"}, str);

namespace fs = std::filesystem;

typedef std::map<std::string, sdbus::Variant> dict;
typedef struct {
    std::unique_ptr<sdbus::IObject> object;
    std::string name;
    dict config;
} application;

dict getConfig(std::string path) {
    std::ifstream initialConfig(path, std::ios::binary);
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
    if (!jsonObj["Timeout"].is_number_unsigned() || !jsonObj["TimeoutPhrase"].is_string())
        DBUS_ERROR("The types of parameter values do not match the required ones in the configuration file");
    return { { "Timeout", sdbus::Variant(static_cast<uint32_t>(jsonObj["Timeout"])) },
             { "TimeoutPhrase", sdbus::Variant(static_cast<std::string>(jsonObj["TimeoutPhrase"])) } };
}

void generateObject(sdbus::IConnection* connection, const std::string& destDir, const std::string& applicationName) {
    sdbus::ObjectPath objectPath{"/com/system/configurationManager/Application/" + applicationName};
    auto app = new application{ sdbus::createObject(*connection, std::move(objectPath)), applicationName, { } };

    auto getConfiguration = [app, &destDir]() {
        if (app->config.empty())
            app->config = getConfig(destDir + "/" + app->name + ".json");
        return app->config;
    };

    auto changeConfiguration = [app, &destDir](const std::string& key, const sdbus::Variant& value) {
        if (key != "Timeout" && key != "TimeoutPhrase")
            DBUS_ERROR("Unknown key transmitted");
        if (key == "Timeout" && !value.containsValueOfType<uint32_t>() ||
            key == "TimeoutPhrase" && !value.containsValueOfType<std::string>())
            DBUS_ERROR("Invalid parameter value type");
        if (app->config.empty())
            app->config = getConfig(destDir + "/" + app->name + ".json");
        app->config[key] = value;
        app->object->emitSignal("configurationChanged")
                    .onInterface("com.system.configurationManager.Application.Configuration")
                    .withArguments(app->config);
        return app->config;
    };

    app->object->addVTable(sdbus::registerMethod("ChangeConfiguration").implementedAs(std::move(changeConfiguration)),
                           sdbus::registerMethod("GetConfiguration").implementedAs(std::move(getConfiguration)),
                           sdbus::registerSignal("configurationChanged").withParameters<dict>())
                .forInterface("com.system.configurationManager.Application.Configuration");
}

int main(int argc, char *argv[]) {
    if (!getenv("HOME"))  {
        std::cerr << "Unable to access current user's home directory" << std::endl;
        return 1;
    }
    std::string destDir = std::string(getpwuid(getuid())->pw_dir) + "/com.system.configurationManager";
    if (!fs::exists(destDir) || !fs::is_directory(destDir)) {
        std::cerr << "Directory \"" << destDir << "\" does not exist" << std::endl;
        return 1;
    }

    sdbus::ServiceName serviceName{"com.system.configurationManager"};
    auto connection = sdbus::createSessionBusConnection(std::move(serviceName));

    uint32_t configFilesNumber = 0, errorsDbusObjectNumber = 0;
    for (const auto& entry : fs::directory_iterator(destDir)) {
        std::string path = entry.path();
        if (fs::is_directory(path))
            continue;
        size_t indexSlashPlus = path.rfind('/') + 1, indexDot = path.rfind(".json");
        if (indexDot == std::string::npos || indexDot != path.size()-5 || indexDot == indexSlashPlus)
            continue;
        configFilesNumber += 1;

        std::string applicationName = path.substr(indexSlashPlus, indexDot - indexSlashPlus);
        try {
            generateObject(connection.get(), destDir, applicationName);
        }
        catch (const sdbus::Error& e) {
             std::cerr << "Error creating dbus-object for application \"" << applicationName << "\"" << std::endl;
             errorsDbusObjectNumber += 1;
        }
    }

    if (!configFilesNumber)
        std::cout << "Configuration files not found" << std::endl;
    else if (configFilesNumber - errorsDbusObjectNumber) {
        std::cout << "Service launched successfully" << std::endl;
        connection->enterEventLoop();
    }
    return 0;
}
