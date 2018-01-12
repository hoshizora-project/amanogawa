#ifndef AMANOGAWA_PLUGIN_H
#define AMANOGAWA_PLUGIN_H

#include "amanogawa/include/config.h"
#include "amanogawa/include/util.h"
#include <string>

namespace amanogawa {
namespace plugin {
struct Plugin {
  const Config config;
  virtual std::string plugin_name() const = 0;
  virtual std::string plugin_full_name() const = 0;

  // TODO: Change top-level config type, std::shared_ptr<cpptoml::table> (?)
  Plugin(const Config &config) : config(config) {}
};
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_PLUGIN_H
