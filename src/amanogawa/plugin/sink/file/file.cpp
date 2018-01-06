#include "amanogawa/core/column_info.h"
#include "amanogawa/core/confing.h"
#include "amanogawa/include/sink_plugin.h"
#include <arrow/api.h>
#include <fstream>
#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin : SinkPlugin {
  std::string plugin_name() const override { return "file"; }
  const logger_t logger = get_logger(plugin_full_name());
  const core::Config::config_map plugin_config;

  explicit SinkFilePlugin(const core::Config &config)
      : SinkPlugin(config),
        plugin_config(sink_config->get_table(plugin_name())) {}

  // TODO: Report exit status
  void drain(const std::shared_ptr<arrow::Table> &data) const override {
    logger->info("drain");

    // std::vector<std::string> col_names = {"id", "name"};
    // std::vector<std::string> col_types = {"int", "string"};
    // core::ColumnsInfo cols_info;
    // for (size_t col_idx = 0; col_idx < 2; ++col_idx) {
    //  cols_info.emplace_back(core::ColumnInfo(
    //      col_names[col_idx], core::type_map.at(col_types[col_idx]),
    //      col_idx));
    //}

    // std::ofstream output(file_name);
    // for (const auto &row : data) {
    //  for (size_t i = 0, end = cols_info.size(); i < end; ++i) {
    //    core::visit(row, cols_info, i, [&](auto el) { output << el; });
    //  }
    //}
    // output.close();
  }
};

extern "C" get_sink_plugin_return_t
get_sink_plugin(const core::Config &config) {
  return std::make_unique<SinkFilePlugin>(config);
}
} // namespace file
} // namespace sink
} // namespace plugin
} // namespace amanogawa