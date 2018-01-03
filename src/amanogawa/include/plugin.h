#ifndef AMANOGAWA_PLUGIN_H
#define AMANOGAWA_PLUGIN_H

#include "util.h"
#include <string>

namespace amanogawa {
namespace plugin {
struct Plugin {
  virtual std::string plugin_name() const = 0;
  virtual std::string plugin_full_name() const = 0;
};
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_PLUGIN_H
