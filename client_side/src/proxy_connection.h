#ifndef PROXY_CONNECTION_H
#define PROXY_CONNECTION_H

#include <map>
#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include <string>

#define SERVICE_NAME "com.system.configurationManager"
#define SERVICE_NAME_FOR_PATH "/com/system/configurationManager"
#define APP_NAME "confManagerApplication1"

typedef std::map<std::string, sdbus::Variant> dict;

class ProxyConnection {
public:
  ProxyConnection(uint32_t &timeout, std::string &timeoutPhrase);
  ~ProxyConnection() = default;

private:
  void changeConfiguration(const dict &config);

  uint32_t &_timeout;
  std::string &_timeoutPhrase;
  std::unique_ptr<sdbus::IProxy> _proxy;
};

#endif
