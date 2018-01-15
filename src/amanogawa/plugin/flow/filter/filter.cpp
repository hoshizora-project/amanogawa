#include "amanogawa/include/flow_plugin.h"
#include "amanogawa/include/util.h"

namespace amanogawa {
namespace plugin {
namespace flow {
namespace filter {
struct FlowFilterPlugin : FlowPlugin {
  std::string plugin_name() const override { return "filter"; }
  const logger_t logger = get_logger(FlowPlugin::plugin_full_name());

  explicit FlowFilterPlugin(const std::string &id, const config_t &config)
      : FlowPlugin(id, config) {}

  std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &data) const override {
    return data; // TMP: pass-through
  }
};

__attribute__((visibility("default"))) extern "C" get_flow_plugin_return_t
get_flow_plugin(const Config &config) {
  return std::make_unique<FlowFilterPlugin>(config);
}
} // namespace filter
} // namespace flow
} // namespace plugin
} // namespace amanogawa