#include "amanogawa/core/column_info.h"
#include "amanogawa/core/confing.h"
#include "amanogawa/core/row.h"
#include "amanogawa/include/source_plugin.h"
#include "text-csv/include/text/csv/istream.hpp"
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {
using amanogawa::plugin::SourcePlugin;

struct SourceFilePlugin : SourcePlugin {
  const logger_t logger = get_logger(plugin_full_name());

  std::string plugin_name() const override { return "file"; }

  const core::Config entire_config;
  const core::Config::config_map config;
  core::ColumnsInfo cols_info;

  explicit SourceFilePlugin(const core::Config &config)
      : entire_config(config),
        config(entire_config.source->get_table(plugin_name())) {
    const auto cols =
        config.source->get_table_array_qualified("format.csv.columns");
    size_t idx = 0;
    for (const auto &col : *cols) {
      cols_info.emplace_back(core::ColumnInfo(
          *(col->get_as<std::string>("name")),
          core::type_map.at(*(col->get_as<std::string>("type"))), idx++));
    }
  }

  std::vector<core::Row> spring() const override {
    logger->info("spring");

    std::vector<core::Row> result;

    const auto file_name = *config->get_as<std::string>("path");
    std::ifstream fs(file_name);
    text::csv::csv_istream csvs(fs);
    const auto num_cols = cols_info.size();

    while (csvs) {
      core::Row row;
      row.reserve(num_cols);
      for (const auto &col_info : cols_info) {
        const auto type = col_info.type;
        if (type == typeid(int)) {
          int i;
          csvs >> i;
          row.emplace_back(i);
        } else if (type == typeid(double)) {
          double d;
          csvs >> d;
          row.emplace_back(d);
        } else if (type == typeid(std::string)) {
          std::string s;
          csvs >> s;
          row.emplace_back(s);
        } else {
        }
      }
      result.emplace_back(row);
    }

    return result;
  }
};

extern "C" get_source_plugin_return_t
get_source_plugin(const core::Config &config) {
  return std::make_unique<SourceFilePlugin>(config);
}
} // namespace file
} // namespace source
} // namespace plugin
} // namespace amanogawa