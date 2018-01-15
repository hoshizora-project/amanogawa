#ifndef AMANOGAWA_CONFIG_BUILDER_H
#define AMANOGAWA_CONFIG_BUILDER_H

#include <cpptoml.h>
#include <string>

namespace amanogawa {
struct ConfigBuilder {
  cpptoml::table config;

  void add(const std::string &key, const std::string &value) {
    config.insert(key, value);
  }
};
} // namespace amanogawa

#endif // AMANOGAWA_CONFIG_BUILDER_H
