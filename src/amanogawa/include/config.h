#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "amanogawa/include/util.h"
#include <cpptoml.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace amanogawa {
namespace toml = cpptoml;
struct Config {
  using table_t = std::shared_ptr<toml::table>;
  const table_t table;
  static const logger_t logger;
  explicit Config(const table_t &table) : table(table) {}
  static std::shared_ptr<Config> from_file(const std::string &file_name) {
    return std::make_shared<Config>(toml::parse_file(file_name));
  }

  // tiny helpers
  std::unordered_map<std::string,
                     std::unordered_map<std::string, std::vector<std::string>>>
  read_plugins() const {
    std::unordered_map<
        std::string, std::unordered_map<std::string, std::vector<std::string>>>
        result;
    const auto root_keys = table->get_keys();
    for (const auto &root_key : root_keys) {
      std::unordered_map<std::string, std::vector<std::string>>
          each_plugin_type;
      const auto keys = table->get_table(root_key)->get_keys();
      for (const auto &id : keys) {
        const auto type =
            *(table->get_qualified_as<std::string>(root_key + "." + id));
        if (each_plugin_type.count(type)) {
          each_plugin_type[type].emplace_back(id);
        } else {
          std::vector<std::string> ids = {id};
          each_plugin_type.emplace(type, ids);
        }
      }
      result.emplace(root_key, each_plugin_type);
    }
    return result;
  }
  table_t get_by_id(const std::string &id) const {
    const auto clazz_keys = table->get_keys();
    for (const auto &clazz_key : clazz_keys) {
      const auto clazz = table->get_table(clazz_key);
      const auto id_keys = clazz->get_keys();
      for (const auto &id_key : id_keys) {
        if (id_key == id) {
          return clazz->get_table(id);
        }
      }
    }
    throw std::runtime_error(id + " not found");
  }
};
const logger_t Config::logger = get_logger("config");
using config_t = std::shared_ptr<Config>;
} // namespace amanogawa

#endif