#include "nlohmann/json.hpp"
#include "proxy_connection.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <thread>
#include <unistd.h>

typedef std::map<std::string, sdbus::Variant> dict;

void init(uint32_t &timeout, std::string &timeoutPhrase) {
  bool errors = !getenv("HOME");
  std::ifstream initialConfig;
  if (!errors) {
    initialConfig.open(std::string(getpwuid(getuid())->pw_dir) +
                           ("/" SERVICE_NAME "/" APP_NAME ".json"),
                       std::ios::binary);
    errors = !initialConfig;
  }

  nlohmann::json jsonObj;
  if (!errors) {
    try {
      initialConfig >> jsonObj;
      errors = !jsonObj.contains("Timeout") ||
               !jsonObj.contains("TimeoutPhrase") ||
               !jsonObj["Timeout"].is_number_unsigned() ||
               !jsonObj["TimeoutPhrase"].is_string();
    } catch (nlohmann::json::parse_error &ex) {
      errors = true;
    }
  }

  if (errors) {
    timeout = 1000;
    timeoutPhrase = "Default text";
  } else {
    timeout = jsonObj["Timeout"];
    timeoutPhrase = jsonObj["TimeoutPhrase"];
  }
}

int main() {
  uint32_t timeout;
  std::string timeoutPhrase;
  init(timeout, timeoutPhrase);

  std::unique_ptr<ProxyConnection> proxyConnection;
  bool connected = false;
  while (true) {
    if (!connected) {
      try {
        proxyConnection =
            std::make_unique<ProxyConnection>(timeout, timeoutPhrase);
        connected = true;
      } catch (const sdbus::Error &e) {
      }
    }
    std::cout << timeoutPhrase << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
  }

  return 0;
}
