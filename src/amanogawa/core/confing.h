#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include <unordered_map>
#include "cpptoml/include/cpptoml.h"

namespace amanogawa {
namespace core {
struct Config {
  const std::shared_ptr<cpptoml::table> root;
  const std::shared_ptr<cpptoml::table> source;
  const std::shared_ptr<cpptoml::table> flow;
  const std::shared_ptr<cpptoml::table> sink;

  explicit Config(const std::shared_ptr<cpptoml::table> &config)
      : root(config),
      source(config->get_table("source")),
      flow(config->get_table("flow")),
      sink(config->get_table("sink")){}
};


Config load_config(const std::string &file_name) {
  const auto config = Config(cpptoml::parse_file(file_name));
  const auto keys = config.source->get_table("format")->get_keys();
  return config;
}
}
}

#endif