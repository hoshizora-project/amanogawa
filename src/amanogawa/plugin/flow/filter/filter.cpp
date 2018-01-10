#include "amanogawa/include/flow_plugin.h"
#include "amanogawa/include/util.h"

namespace amanogawa {
namespace plugin {
namespace flow {
namespace filter {
struct FlowFilterPlugin : FlowPlugin {
  std::string plugin_name() const override { return "filter"; }
  const logger_t logger = get_logger(FlowPlugin::plugin_full_name());
  const core::Config::config_map plugin_config;

  explicit FlowFilterPlugin(const core::Config &config)
      : FlowPlugin(config),
        plugin_config(flow_config->get_table(plugin_name())) {}

  std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &data) const override {
    return data; // TMP: pass-through
  }
};

extern "C" get_flow_plugin_return_t
get_flow_plugin(const core::Config &config) {
  return std::make_unique<FlowFilterPlugin>(config);
}
} // namespace filter
} // namespace flow
} // namespace plugin
} // namespace amanogawa