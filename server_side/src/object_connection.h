#ifndef OBJECT_CONNECTION_H
#define OBJECT_CONNECTION_H

#include "nlohmann/json.hpp"
#include <map>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include <string>

#define SERVICE_NAME "com.system.configurationManager"
#define SERVICE_NAME_FOR_PATH "/com/system/configurationManager"

typedef std::map<std::string, sdbus::Variant> dict;

class ObjectConnection {
public:
  ObjectConnection(sdbus::IConnection *connection, const std::string &path,
                   const std::string &appName);
  ~ObjectConnection() = default;

private:
  void changeConfiguration(const std::string &key, const sdbus::Variant &value);
  dict getConfiguration();

  void setConfig();

  std::string configPath;
  dict config;
  std::unique_ptr<sdbus::IObject> object;
};

#endif
