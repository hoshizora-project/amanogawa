#include "amanogawa/include/api.h"
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
  const logger_t logger = get_logger(SinkPlugin::plugin_full_name());
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
    const auto delimiter =
        *sink_config->get_qualified_as<std::string>("format.csv.delimiter");
    text::csv::csv_ostream csv_os(fs, delimiter[0]);

    const auto write_header = plugin_config->get_as<bool>("write_header");
    if (write_header.value_or(true)) {
      for (const auto &field : schema->fields()) {
        csv_os << field->name();
      }
      csv_os << text::csv::endl;
    }

    for (size_t i = 0, end = data->num_rows(); i < end; ++i) {
      for (const auto &field : schema->fields()) {
        auto field_name = field->name();

        const auto field_idx = data->schema()->GetFieldIndex(field_name);
        const auto col = data->column(static_cast<int>(field_idx));
        const auto type = col->type()->id();

        for (const auto &chunk : col->data()->chunks()) {
          if (type == arrow::Int32Type::type_id) {
            const auto int32_chunk =
                std::static_pointer_cast<arrow::Int32Array>(chunk);
            csv_os << int32_chunk->Value(i);
          } else if (type == arrow::DoubleType::type_id) {
            const auto float64_chunk =
                std::static_pointer_cast<arrow::DoubleArray>(chunk);
            csv_os << float64_chunk->Value(i);
          } else if (type == arrow::StringType::type_id) {
            const auto utf8_chunk =
                std::static_pointer_cast<arrow::StringArray>(chunk);
            csv_os << utf8_chunk->GetString(i);
          } else {
            logger->warn("Detected unsupported type: {}", type);
          }
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