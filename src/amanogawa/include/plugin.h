#ifndef AMANOGAWA_PLUGIN_H
#define AMANOGAWA_PLUGIN_H

#include "amanogawa/include/util.h"
#include "amanogawa/include/config.h"
#include <string>

namespace amanogawa {
namespace plugin {
struct Plugin {
  const core::Config config;
  virtual std::string plugin_name() const = 0;
  virtual std::string plugin_full_name() const = 0;

  // TODO: Change top-level config type, std::shared_ptr<cpptoml::table> (?)
  Plugin(const core::Config &config) : config(config) {}
};
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_PLUGIN_H
