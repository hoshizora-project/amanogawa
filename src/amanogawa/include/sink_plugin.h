#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <memory>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
struct SinkPlugin : Plugin {
  std::string plugin_full_name() const override {
    return "sink_" + plugin_name();
  }
  const std::string from;

  virtual void *drain(const std::shared_ptr<arrow::Table> &data) const = 0;

  SinkPlugin(const std::string &id, const std::string &from,
             const config_t &config)
      : Plugin(id, config), from(from) {}
};

using sink_plugin_t = std::shared_ptr<SinkPlugin>;
using get_sink_plugin_t = sink_plugin_t (*)(const std::string &,
                                            const std::string &,
                                            const config_t &);
sink_plugin_t as_sink(const plugin_t &plugin) {
  return std::dynamic_pointer_cast<SinkPlugin>(plugin);
}
} // namespace plugin
} // namespace amanogawa

#endif