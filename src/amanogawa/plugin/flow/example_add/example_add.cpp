#include "amanogawa/core/confing.h"
#include "amanogawa/include/flow_plugin.h"
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
struct FlowExampleAddPlugin : FlowPlugin {
  std::string plugin_name() const override { return "example_add"; }
  const logger_t logger = get_logger(plugin_full_name());
  const core::Config::config_map plugin_config;

  explicit FlowExampleAddPlugin(const core::Config &config)
      : FlowPlugin(config),
        plugin_config(flow_config->get_table(plugin_name())) {}

  std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &data) const override {
    logger->info("flow");

    // const auto rename_rules = *flow_config->get_table_array("rename");
    // for(const auto &rename_rule:rename_rules){
    //  const auto from = rename_rule->get_as<std::string>("from");
    //  const auto to = rename_rule->get_as<std::string>("to");
    //  schema
    //}

    // auto id_col = data->column(data->schema()->GetFieldIndex("id"));
    // for (const auto &id_chunk : id_col->data()->chunks()) {
    //  const auto chunk =
    //  std::dynamic_pointer_cast<arrow::Int32Array>(id_chunk); const auto
    //  length = id_chunk->length(); for (size_t i = 0; i < length; ++i) {
    //    logger->info("array[{}]: {}", i, chunk->Value(i));
    //  }
    //}

    return data;
  }
};

extern "C" get_flow_plugin_return_t
get_flow_plugin(const core::Config &config) {
  return std::make_unique<FlowExampleAddPlugin>(config);
}
} // namespace example_add
} // namespace flow
} // namespace plugin
} // namespace amanogawa
