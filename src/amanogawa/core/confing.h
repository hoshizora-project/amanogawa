#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "amanogawa/core/util.h"
#include "cpptoml/include/cpptoml.h"

namespace amanogawa {
namespace core {
class Config {
public:
  const std::shared_ptr<cpptoml::table> root;
  const std::shared_ptr<cpptoml::table> source;
  const std::shared_ptr<cpptoml::table> flow;
  const std::shared_ptr<cpptoml::table> sink;

  static Config load_from_file(const std::string &file_name) {
    const auto config = Config(cpptoml::parse_file(file_name));
    const auto valid = validate_minimal_requirements(config);
    if (!valid) {
      logger->error("invalid config");
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
} // namespace core
} // namespace amanogawa

#endif