#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "amanogawa/include/util.h"
#include <cpptoml.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
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
  // [class, [type, [id, from]]]
  using plugin_meta_t = std::unordered_map<
      std::string,
      std::unordered_map<
          std::string,
          std::vector<std::pair<std::string, std::vector<std::string>>>>>;
  plugin_meta_t read_plugins() const {
    plugin_meta_t result;
    const auto clazzes = table->get_keys();
    for (const auto &clazz : clazzes) {
      std::unordered_map<
          std::string,
          std::vector<std::pair<std::string, std::vector<std::string>>>>
          each_plugin_type;
      const auto class_table = table->get_table(clazz);
      const auto ids = class_table->get_keys();
      for (const auto &id : ids) {
        const auto type = *class_table->get_as<std::string>(id);
        const auto from =
            clazz == string::clazz::_source
                ? std::vector<std::string>{}
                : clazz == string::clazz::_flow
                      ? std::vector<std::string>{*class_table
                                                      ->get_as<std::string>(
                                                          "from")}
                      : clazz == string::clazz::_sink
                            ? std::vector<std::string>{*class_table->get_as<
                                  std::string>("from")}
                            : std::vector<std::string>{};
        const auto pair = std::make_pair(id, from);
        if (each_plugin_type.count(type)) {
          each_plugin_type[type].emplace_back(pair);
        } else {
          std::vector<std::pair<std::string, std::vector<std::string>>>
              id_list = {pair};
          each_plugin_type.emplace(type, id_list);
        }
      }
      result.emplace(clazz, each_plugin_type);
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