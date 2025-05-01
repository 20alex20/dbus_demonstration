#include "proxy_connection.h"

ProxyConnection::ProxyConnection(uint32_t &timeout, std::string &timeoutPhrase)
    : _timeout(timeout), _timeoutPhrase(timeoutPhrase) {
  sdbus::ServiceName serviceName{SERVICE_NAME};
  sdbus::ObjectPath objectPath{SERVICE_NAME_FOR_PATH "/Application/" APP_NAME};
  _proxy = sdbus::createProxy(serviceName, std::move(objectPath));

  _proxy->uponSignal("configurationChanged")
      .onInterface(SERVICE_NAME ".Application.Configuration")
      .call([this](const dict &config) { this->changeConfiguration(config); });
}

void ProxyConnection::changeConfiguration(const dict &config) {
  dict::const_iterator search;
  if ((search = config.find("Timeout")) != config.end() &&
      search->second.containsValueOfType<uint32_t>())
    _timeout = search->second.get<uint32_t>();
  if ((search = config.find("TimeoutPhrase")) != config.end() &&
      search->second.containsValueOfType<std::string>())
    _timeoutPhrase = search->second.get<std::string>();
}
