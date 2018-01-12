#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "amanogawa/include/util.h"
#include "cpptoml/include/cpptoml.h"

namespace amanogawa {
class Config {
public:
  using config_map = std::shared_ptr<cpptoml::table>;
  const config_map root;
  const config_map source;
  const config_map flow;
  const config_map sink;

  static Config load_from_file(const std::string &file_name) {
    const auto config = Config(cpptoml::parse_file(file_name));
    const auto valid = validate_minimal_requirements(config);
    if (!valid) {
      logger->error("invalid entire_config");
    }
    return config;
  }

private:
  static const logger_t logger;

  explicit Config(const std::shared_ptr<cpptoml::table> &config)
      : root(config), source(config->get_table("source")),
        flow(config->get_table("flow")), sink(config->get_table("sink")) {}

  static bool validate_minimal_requirements(const Config &config) {
    return config.source != nullptr && config.sink != nullptr &&
           !config.source->get_keys().empty() &&
           !config.sink->get_keys().empty();
  }
};
const logger_t Config::logger = get_logger("config");
} // namespace amanogawa

#endif