#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SinkPlugin : Plugin {
  const Config::config_map sink_config;
  std::string plugin_full_name() const override {
    return "sink_" + plugin_name();
  }

  virtual void drain(const std::shared_ptr<arrow::Table> &data) const = 0;

  SinkPlugin(const Config &config)
      : Plugin(config), sink_config(config.sink) {}
};

using get_sink_plugin_return_t = std::unique_ptr<SinkPlugin>;
using get_sink_plugin_t = get_sink_plugin_return_t (*)(const Config &);
} // namespace plugin
} // namespace amanogawa

#endif