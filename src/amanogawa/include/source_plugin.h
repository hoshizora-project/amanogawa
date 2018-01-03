#ifndef AMANOGAWA_SOURCE_H
#define AMANOGAWA_SOURCE_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include "amanogawa/include/plugin.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SourcePlugin : Plugin {
  std::string plugin_full_name() const {
      return "source_" + plugin_name();
  }

  virtual std::vector<core::Row> spring() const = 0;
};

using get_source_plugin_return_t = std::unique_ptr<SourcePlugin>;
using get_source_plugin_t =
    get_source_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif