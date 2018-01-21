#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <fstream>
#include <iostream>
#include <text/csv/istream.hpp>
#include <text/csv/ostream.hpp>
#include <text/csv/rows.hpp>

namespace amanogawa {
namespace plugin {
namespace format {
namespace csv {

struct FormatCsvPlugin : FormatPlugin {
  std::string plugin_name() const override { return "csv"; }

  std::shared_ptr<arrow::Schema> schema;

  // TODO: Validate config
  explicit FormatCsvPlugin(const std::string &id, const config_t &config)
      : FormatPlugin(id, config) {
    init_logger();

    const auto cols = this->config->get_table_array(string::keyword::columns);
    if (cols != nullptr) {
      std::vector<std::shared_ptr<arrow::Field>> fields;
      for (const auto &col : *cols) {
        fields.emplace_back(std::make_shared<arrow::Field>(
            *col->get_as<std::string>(string::keyword::name),
            get_arrow_data_type(
                *col->get_as<std::string>(string::keyword::type))));
      }
      schema = arrow::schema(fields);
    }
  }

  // TODO: Want to use https://issues.apache.org/jira/browse/ARROW-25
  std::shared_ptr<arrow::Table> parse(const std::string &path) const override {
    logger->info("parse");

    std::ifstream fs(path);
    const auto delimiter =
        config->get_as<std::string>("delimiter").value_or(",");
    text::csv::csv_istream csv_is(fs, delimiter[0]);

    const auto num_fields = static_cast<size_t>(schema->num_fields());

    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    const auto skip_header = config->get_as<bool>("skip_header");
    if (skip_header.value_or(false)) {
      while (csv_is.line_number() == 1) {
        std::string devnull;
        csv_is >> devnull;
      }
    }

    // FIXME: Manage invalid row
    // FIXME: Extra invalid row??? (mnist)
    // TMP: Accept only complete config of columns
    while (csv_is) {
      for (size_t i = 0; i < num_fields; ++i) {
        const auto type = schema->field(static_cast<int>(i))->type()->id();
        // FIXME: Use visitor or pre-planing (concat field info for each row)
        if (type == arrow::Int32Type::type_id) {
          int val;
          csv_is >> val;
          std::static_pointer_cast<arrow::Int32Builder>(builders.at(i))
              ->Append(val);
        } else if (type == arrow::DoubleType::type_id) {
          double val;
          csv_is >> val;
          std::static_pointer_cast<arrow::DoubleBuilder>(builders.at(i))
              ->Append(val);
        } else if (type == arrow::StringType::type_id) {
          std::string val;
          csv_is >> val;
          std::static_pointer_cast<arrow::StringBuilder>(builders.at(i))
              ->Append(val);
        } else {
          logger->warn("Detected unsupported type: {}", type);
        }
      }
    }

    std::vector<std::shared_ptr<arrow::Column>> columns;
    columns.reserve(num_fields);
    for (size_t i = 0; i < num_fields; ++i) {
      std::shared_ptr<arrow::Array> array;
      builders.at(i)->Finish(&array);
      columns.emplace_back(std::make_shared<arrow::Column>(
          schema->field(static_cast<int>(i)), array));
    }

    return arrow::Table::Make(schema, columns);
  }

  void *format(const std::string &path,
               const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("format");

    const auto output_fields =
        schema != nullptr ? schema->fields() : table->schema()->fields();

    std::ofstream fs(path);
    const auto delimiter =
        config->get_as<std::string>("delimiter").value_or(",");
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

__attribute__((visibility("default"))) extern "C" format_plugin_t
get_plugin(const std::string &id, const config_t &config) {
  return std::make_shared<FormatCsvPlugin>(id, config);
}
} // namespace csv
} // namespace format
} // namespace plugin
} // namespace amanogawa