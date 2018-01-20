#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <text/csv/ostream.hpp>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SinkFilePlugin : SinkPlugin {
  std::string plugin_name() const override { return "file"; }
  std::shared_ptr<arrow::Schema> output_schema;

  explicit SinkFilePlugin(const std::string &id, const std::string &from,
                          const config_t &config)
      : SinkPlugin(id, from, config) {
    init_logger();

    // FIXME: Bug in cpptoml; if array of tables is empty, return nullptr
    const auto cols = format_config->get_table_array(string::keyword::columns);
    if (cols != nullptr) {
      std::vector<std::shared_ptr<arrow::Field>> fields;
      for (const auto &col : *cols) {
        fields.emplace_back(std::make_shared<arrow::Field>(
            *col->get_as<std::string>(string::keyword::name),
            get_arrow_data_type(
                *col->get_as<std::string>(string::keyword::type))));
      }
      output_schema = arrow::schema(fields);
    }
  }

  // TODO: Report exit status
  void *drain(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("drain");

    const auto output_fields = output_schema != nullptr
                                   ? output_schema->fields()
                                   : table->schema()->fields();

    const auto file_name = *config->get_as<std::string>("path");
    std::ofstream fs(file_name);
    const auto delimiter = *format_config->get_as<std::string>("delimiter");
    text::csv::csv_ostream csv_os(fs, delimiter[0]);

    const auto write_header = config->get_as<bool>("write_header");
    if (write_header.value_or(true)) {
      for (const auto &field : output_fields) {
        csv_os << field->name();
      }
      csv_os << text::csv::endl;
    }

    // FIXME...
    const auto num_columns = output_fields.size();
    std::vector<uint32_t> col_idxes(num_columns);
    for (size_t i = 0; i < num_columns; ++i) {
      const auto index =
          table->schema()->GetFieldIndex(output_fields.at(i)->name());
      if (index < 0) {
        throw std::runtime_error("Invalid output field: " +
                                 output_fields.at(i)->name());
      }
      col_idxes[i] = static_cast<uint32_t>(index);
    }

    std::vector<size_t> curr_chunk(num_columns);
    std::vector<size_t> curr(num_columns);
    for (size_t i = 0, end = table->num_rows(); i < end; ++i) {
      for (size_t j = 0; j < num_columns; ++j) {
        const auto col = table->column(col_idxes[j]);
        const auto type = col->type()->id();

        if (curr[j] >= col->data()->chunk(curr_chunk[j])->length()) {
          curr[j] = 0;
          curr_chunk[j]++;
        }

        const auto chunk = col->data()->chunk(curr_chunk[j]);
        if (type == arrow::Int32Type::type_id) {
          const auto int32_chunk =
              std::static_pointer_cast<arrow::Int32Array>(chunk);
          csv_os << int32_chunk->Value(curr[j]);
        } else if (type == arrow::DoubleType::type_id) {
          const auto float64_chunk =
              std::static_pointer_cast<arrow::DoubleArray>(chunk);
          csv_os << float64_chunk->Value(curr[j]);
        } else if (type == arrow::StringType::type_id) {
          const auto utf8_chunk =
              std::static_pointer_cast<arrow::StringArray>(chunk);
          csv_os << utf8_chunk->GetString(curr[j]);
        } else {
          logger->warn("Detected unsupported type: {}", type);
        }
        curr[j]++;
      }
      csv_os << text::csv::endl;
    }
    fs.close();

    return nullptr;
  }
};

__attribute__((visibility("default"))) extern "C" sink_plugin_t
get_plugin(const std::string &id, const std::string &from,
           const config_t &config) {
  return std::make_shared<SinkFilePlugin>(id, from, config);
}
} // namespace file
} // namespace sink
} // namespace plugin
} // namespace amanogawa