#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "cpptoml/include/cpptoml.h"
#include <unordered_map>

namespace amanogawa {
namespace core {
struct Config {
  const std::shared_ptr<cpptoml::table> root;
  const std::shared_ptr<cpptoml::table> source;
  const std::shared_ptr<cpptoml::table> flow;
  const std::shared_ptr<cpptoml::table> sink;

  explicit Config(const std::shared_ptr<cpptoml::table> &config)
      : root(config), source(config->get_table("source")),
        flow(config->get_table("flow")), sink(config->get_table("sink")) {}
};

bool validate_minimal_requirements(const Config &config) {
  return config.source != nullptr && config.sink != nullptr &&
         !config.source->get_keys().empty() && !config.sink->get_keys().empty();
}

Config load_config(const std::string &file_name) {
  const auto config = Config(cpptoml::parse_file(file_name));
  const auto valid = validate_minimal_requirements(config);
  if (!valid) {
    printf("invalid config\n");
  }
  return config;
}
} // namespace core
} // namespace amanogawa

#endif