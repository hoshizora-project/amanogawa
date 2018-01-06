#include "amanogawa/core/column_info.h"
#include "amanogawa/core/confing.h"
#include "amanogawa/include/sink_plugin.h"
#include "amanogawa/include/util.h"
#include <arrow/api.h>
#include <fstream>
#include <iostream>
#include <string>
#include <text/csv/ostream.hpp>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin : SinkPlugin {
  std::string plugin_name() const override { return "file"; }
  const logger_t logger = get_logger(plugin_full_name());
  const core::Config::config_map plugin_config;

  std::shared_ptr<arrow::Schema> schema;

  explicit SinkFilePlugin(const core::Config &config)
      : SinkPlugin(config),
        plugin_config(sink_config->get_table(plugin_name())) {
    const auto cols =
        sink_config->get_table_array_qualified("format.csv.columns");

    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col : *cols) {
      fields.emplace_back(std::make_shared<arrow::Field>(
          *(col->get_as<std::string>("name")),
          get_arrow_data_type(*col->get_as<std::string>("type"))));
    }
    schema = arrow::schema(std::move(fields));
  }

  // TODO: Report exit status
  void drain(const std::shared_ptr<arrow::Table> &data) const override {
    logger->info("drain");

    const auto file_name = *plugin_config->get_as<std::string>("path");
    std::ofstream fs(file_name);
    text::csv::csv_ostream csv_os(fs);

    const auto write_header = plugin_config->get_as<bool>("write_header");
    if (write_header.value_or(true)) {
      for (const auto &field : schema->fields()) {
        auto field_name = field->name();

        // TMP: Rename
        if (field_name == "id") {
          field_name = "ID";
        }

        csv_os << field_name;
      }
      csv_os << text::csv::endl;
    }

    for (size_t i = 0, end = data->num_rows(); i < end; ++i) {
      for (const auto &field : schema->fields()) {
        auto field_name = field->name();

        // TMP: Rename
        if (field_name == "ID") {
          field_name = "id";
        }

        const auto field_idx = data->schema()->GetFieldIndex(field_name);
        const auto col = data->column(static_cast<int>(field_idx));

        // TMP: Assume that all arrays consist of a single chunk
        const auto chunk = col->data()->chunk(0);
        const auto type = col->type()->name();

        if (type == "int32") {
          const auto int32_chunk =
              std::dynamic_pointer_cast<arrow::Int32Array>(chunk);
          csv_os << int32_chunk->Value(i);
        } else if (type == "float64") {
          const auto float64_chunk =
              std::dynamic_pointer_cast<arrow::DoubleArray>(chunk);
          csv_os << float64_chunk->Value(i);
        } else if (type == "utf8") {
          const auto utf8_chunk =
              std::dynamic_pointer_cast<arrow::StringArray>(chunk);
          csv_os << utf8_chunk->GetString(i);
        } else {
          logger->warn("Invalid");
        }
      }
      csv_os << text::csv::endl;
    }
    fs.close();
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