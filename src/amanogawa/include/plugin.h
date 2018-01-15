#ifndef AMANOGAWA_PLUGIN_H
#define AMANOGAWA_PLUGIN_H

#include "amanogawa/include/config.h"
#include "amanogawa/include/util.h"
#include <string>

namespace amanogawa {
namespace plugin {
struct Plugin {
  const std::string id;
  const config_t root_config;
  const Config::table_t config;
  virtual std::string plugin_name() const = 0;
  virtual std::string plugin_full_name() const = 0;

  Plugin(const std::string &id, const config_t &config)
      : id(id), root_config(config), config(root_config->get_by_id(id)) {}
};
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_PLUGIN_H
