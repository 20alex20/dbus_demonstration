#include "object_connection.h"
#include <filesystem>
#include <iostream>
#include <list>
#include <pwd.h>
#include <unistd.h>

namespace fs = std::filesystem;

bool isConfigFile(const std::string &path, std::string &appName) {
  if (fs::is_directory(path))
    return false;
  size_t indexSlashPlus = path.rfind('/') + 1, indexDot = path.rfind(".json");
  if (indexDot == std::string::npos || indexDot != path.size() - 5 ||
      indexDot == indexSlashPlus)
    return false;
  appName = path.substr(indexSlashPlus, indexDot - indexSlashPlus);
  return true;
}

int main() {
  if (!getenv("HOME")) {
    std::cerr << "Unable to access current user's home directory" << std::endl;
    return 1;
  }
  std::string destDir =
      std::string(getpwuid(getuid())->pw_dir) + ("/" SERVICE_NAME);
  if (!fs::exists(destDir) || !fs::is_directory(destDir)) {
    std::cerr << "Directory \"" << destDir << "\" does not exist" << std::endl;
    return 1;
  }

  sdbus::ServiceName serviceName{SERVICE_NAME};
  auto connection = sdbus::createSessionBusConnection(serviceName);
  std::list<std::unique_ptr<ObjectConnection>> objectConnections;
  uint32_t configFilesNumber = 0, errorsDbusObjectNumber = 0;
  for (const auto &entry : fs::directory_iterator(destDir)) {
    std::string path = entry.path(), appName;
    if (!isConfigFile(path, appName))
      continue;
    configFilesNumber += 1;

    try {
      objectConnections.push_back(
          std::make_unique<ObjectConnection>(connection.get(), path, appName));
    } catch (const sdbus::Error &e) {
      std::cerr << "Error creating dbus-object for application \"" << appName
                << "\"" << std::endl;
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
