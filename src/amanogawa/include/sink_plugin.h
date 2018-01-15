#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SinkPlugin : Plugin {
  std::string plugin_full_name() const override {
    return "sink_" + plugin_name();
  }

  virtual void *drain(const std::shared_ptr<arrow::Table> &data) const = 0;

  SinkPlugin(const std::string &id, const config_t &config)
      : Plugin(id, config) {}
};

using get_sink_plugin_return_t = std::unique_ptr<SinkPlugin>;
using get_sink_plugin_t = get_sink_plugin_return_t (*)(const std::string &,
                                                       const config_t &);
} // namespace plugin
} // namespace amanogawa

#endif