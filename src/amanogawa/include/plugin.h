#ifndef AMANOGAWA_PLUGIN_H
#define AMANOGAWA_PLUGIN_H

#include "amanogawa/include/config.h"
#include "amanogawa/include/util.h"
#include <string>

namespace amanogawa {
namespace plugin {
struct Plugin {
  logger_t logger;
  const std::string id;
  const config_t root_config;
  const Config::table_t config;
  std::string _plugin_full_name; // for spdlog::drop
  virtual std::string plugin_name() const = 0;
  virtual std::string plugin_full_name() const = 0;

  void init_logger() {
    logger = get_logger(plugin_full_name());
    _plugin_full_name = plugin_full_name();
  }

  virtual ~Plugin() { drop_logger(_plugin_full_name); }

  Plugin(const std::string &id, const config_t &config)
      : id(id), root_config(config), config(root_config->get_by_id(id)) {}
};

using plugin_t = std::shared_ptr<Plugin>;
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_PLUGIN_H
