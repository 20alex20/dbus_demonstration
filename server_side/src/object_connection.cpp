#include "object_connection.h"
#include <fstream>

#define DBUS_ERROR(str)                                                        \
  throw sdbus::Error(sdbus::Error::Name{SERVICE_NAME ".Error"}, str);

ObjectConnection::ObjectConnection(sdbus::IConnection *connection,
                                   const std::string &path,
                                   const std::string &appName)
    : configPath(path), config() {
  sdbus::ObjectPath objectPath{(SERVICE_NAME_FOR_PATH "/Application/") +
                               appName};
  object = sdbus::createObject(*connection, std::move(objectPath));

  object
      ->addVTable(
          sdbus::registerMethod("ChangeConfiguration")
              .implementedAs(
                  [this](const std::string &key, const sdbus::Variant &value) {
                    this->changeConfiguration(key, value);
                  }),
          sdbus::registerMethod("GetConfiguration").implementedAs([this]() {
            return this->getConfiguration();
          }),
          sdbus::registerSignal("configurationChanged").withParameters<dict>())
      .forInterface(SERVICE_NAME ".Application.Configuration");
}

void ObjectConnection::changeConfiguration(const std::string &key,
                                           const sdbus::Variant &value) {
  if (key != "Timeout" && key != "TimeoutPhrase")
    DBUS_ERROR("Unknown key transmitted");
  if ((key == "Timeout" && !value.containsValueOfType<uint32_t>()) ||
      (key == "TimeoutPhrase" && !value.containsValueOfType<std::string>()))
    DBUS_ERROR("Invalid parameter value type");

  if (config.empty())
    setConfig();
  config[key] = value;
  object->emitSignal("configurationChanged")
      .onInterface(SERVICE_NAME ".Application.Configuration")
      .withArguments(config);
}

dict ObjectConnection::getConfiguration() {
  if (config.empty())
    setConfig();
  return config;
}

void ObjectConnection::setConfig() {
  std::ifstream initialConfig(configPath, std::ios::binary);
  if (!initialConfig)
    DBUS_ERROR("Unable to read configuration file");

  nlohmann::json jsonObj;
  try {
    initialConfig >> jsonObj;
  } catch (nlohmann::json::parse_error &ex) {
    DBUS_ERROR("Error parsing config file");
  }

  if (!jsonObj.contains("Timeout") || !jsonObj.contains("TimeoutPhrase"))
    DBUS_ERROR("Not all parameters are listed in the configuration file");
  if (!jsonObj["Timeout"].is_number_unsigned() ||
      !jsonObj["TimeoutPhrase"].is_string())
    DBUS_ERROR("The types of parameter values do not match the required ones "
               "in the configuration file");
  config = {
      {"Timeout", sdbus::Variant(static_cast<uint32_t>(jsonObj["Timeout"]))},
      {"TimeoutPhrase",
       sdbus::Variant(static_cast<std::string>(jsonObj["TimeoutPhrase"]))}};
}
