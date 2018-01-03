#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include "amanogawa/include/plugin.h"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SinkPlugin : Plugin {
  std::string plugin_full_name() const {
    return "sink_" + plugin_name();
  }

  virtual void drain(const std::string &file_name,
                     const std::vector<core::Row> &data) const = 0;
};

using get_sink_plugin_return_t = std::unique_ptr<SinkPlugin>;
using get_sink_plugin_t = get_sink_plugin_return_t (*)(const core::Config &);
} // namespace plugin
} // namespace amanogawa

#endif