#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <text/csv/istream.hpp>

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {

struct SourceFilePlugin : SourcePlugin {
  std::string plugin_name() const override { return "file"; }
  const logger_t logger = get_logger(SourcePlugin::plugin_full_name());

  std::shared_ptr<arrow::Schema> schema;

  // TODO: Validate config
  explicit SourceFilePlugin(const std::string &id, const config_t &config)
      : SourcePlugin(id, config) {
    const auto format =
        *format_config->get_as<std::string>(string::keyword::type);
    const auto cols = format_config->get_table_array(string::keyword::columns);
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col : *cols) {
      fields.emplace_back(std::make_shared<arrow::Field>(
          *col->get_as<std::string>(string::keyword::name),
          get_arrow_data_type(
              *col->get_as<std::string>(string::keyword::type))));
    }
    schema = arrow::schema(fields);
  }

  // TODO: Want to use https://issues.apache.org/jira/browse/ARROW-25
  std::shared_ptr<arrow::Table> spring() const override {
    logger->info("spring");

    const auto file_name = *config->get_as<std::string>("path");
    std::ifstream fs(file_name);
    const auto delimiter =
        *format_config->get_qualified_as<std::string>("delimiter");
    text::csv::csv_istream csv_is(fs, delimiter[0]);

    const auto num_fields = static_cast<size_t>(schema->num_fields());

    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    const auto skip_header = format_config->get_as<bool>("skip_header");
    if (skip_header.value_or(false)) {
      while (csv_is.line_number() == 1) {
        std::string devnull;
        csv_is >> devnull;
      }
    }

    // FIXME: Manage invalid row
    // FIXME: Extra invalid row??? (mnist)
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
};

__attribute__((visibility("default"))) extern "C" source_plugin_t
get_plugin(const std::string &id, const config_t &config) {
  return std::make_shared<SourceFilePlugin>(id, config);
}
} // namespace file
} // namespace source
} // namespace plugin
} // namespace amanogawa