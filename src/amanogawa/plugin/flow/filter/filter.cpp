#include "amanogawa/include/flow_plugin.h"
#include "amanogawa/include/util.h"
#include <memory>
#include <unordered_map>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace filter {
struct FlowFilterPlugin : FlowPlugin {
  std::string plugin_name() const override { return "filter"; }

  explicit FlowFilterPlugin(const std::string &id, const std::string &from,
                            const config_t &config)
      : FlowPlugin(id, from, config) {
    init_logger();
  }

  std::shared_ptr<
      std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>
  flow(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("flow");

    const auto result = std::make_shared<
        std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>();
    result->emplace(id, table);
    return result; // TMP: pass-through
  }
};

extern "C" flow_plugin_t get_plugin(const std::string &id,
                                    const std::string &from,
                                    const config_t &config) {
  return std::make_shared<FlowFilterPlugin>(id, from, config);
}
} // namespace filter
} // namespace flow
} // namespace plugin
} // namespace amanogawa