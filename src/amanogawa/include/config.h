#ifndef AMANOGAWA_CONF_H
#define AMANOGAWA_CONF_H

#include "amanogawa/include/common.h"
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

    // nested
    for (const auto &clazz_key : clazz_keys) {
      const auto clazz = table->get_table(clazz_key);
      const auto id_keys = clazz->get_keys();
      for (const auto &id_key : id_keys) {
        const auto type = clazz->get_table(id_key);
        const auto item_keys = type->get_keys();
        for (const auto &item_key : item_keys) {
          if (id_key + "." + item_key == id) {
            return type->get_table(item_key);
          }
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