#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <exception>
#include <fstream>
#include <json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
struct SourceJsonPlugin : SourcePlugin {
  std::string plugin_name() const override { return "json"; }

  std::shared_ptr<arrow::Schema> schema;

  explicit SourceJsonPlugin(const std::string &id, const config_t &config)
      : SourcePlugin(id, config) {
    init_logger();

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

  inline bool ends_with(std::string const &value,
                        std::string const &ending) const {
    if (ending.size() > value.size())
      return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
  }

  std::shared_ptr<arrow::Table> spring() const override {
    logger->info("spring");

    using json = nlohmann::json;

    const auto file_name = *config->get_as<std::string>("path");
    std::ifstream fs(file_name);

    const auto num_fields = static_cast<size_t>(schema->num_fields());
    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    const auto is_jsonl = ends_with(file_name, ".jsonl") ||
                          config->get_as<bool>("jsonl").value_or(false);
    if (!is_jsonl) {
      json parsed;
      fs >> parsed;
      for (json::iterator it = parsed.begin(); it != parsed.end(); ++it) {
        for (size_t i = 0; i < num_fields; ++i) {
          const auto field = schema->field(i);
          const auto type = field->type()->id();
          const auto key = field->name();

          const auto val = it.value()[key];
          if (type == arrow::Int32Type::type_id) {
            std::static_pointer_cast<arrow::Int32Builder>(builders.at(i))
                ->Append(val.get<int32_t>());
          } else if (type == arrow::DoubleType::type_id) {
            std::static_pointer_cast<arrow::DoubleBuilder>(builders.at(i))
                ->Append(val.get<double>());
          } else if (type == arrow::StringType::type_id) {
            std::static_pointer_cast<arrow::StringBuilder>(builders.at(i))
                ->Append(val.get<std::string>());
          } else {
            logger->warn("Detected unsupported type: {}", type);
          }
        }
      }
    } else {
      std::string line;
      while (std::getline(fs, line)) {
        const auto parsed = json::parse(line);
        for (size_t i = 0; i < num_fields; ++i) {
          const auto field = schema->field(i);
          const auto type = field->type()->id();
          const auto key = field->name();

          if (type == arrow::Int32Type::type_id) {
            std::static_pointer_cast<arrow::Int32Builder>(builders.at(i))
                ->Append(parsed[key].get<int32_t>());
          } else if (type == arrow::DoubleType::type_id) {
            std::static_pointer_cast<arrow::DoubleBuilder>(builders.at(i))
                ->Append(parsed[key].get<double>());
          } else if (type == arrow::StringType::type_id) {
            std::static_pointer_cast<arrow::StringBuilder>(builders.at(i))
                ->Append(parsed[key].get<std::string>());
          } else {
            logger->warn("Detected unsupported type: {}", type);
          }
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
  return std::make_shared<SourceJsonPlugin>(id, config);
}
} // namespace file
} // namespace sink
} // namespace plugin
} // namespace amanogawa