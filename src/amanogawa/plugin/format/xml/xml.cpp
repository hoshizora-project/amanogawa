#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <pugixml.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace format {
namespace xml {
struct FormatXmlPlugin : FormatPlugin {
  std::string plugin_name() const override { return "xml"; }

  std::shared_ptr<arrow::Schema> schema;

  explicit FormatXmlPlugin(const std::string &id, const config_t &config)
      : FormatPlugin(id, config) {
    init_logger();

    const auto format =
        *this->config->get_as<std::string>(string::keyword::type);
    const auto cols = this->config->get_table_array(string::keyword::columns);
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col : *cols) {
      fields.emplace_back(std::make_shared<arrow::Field>(
          *col->get_as<std::string>(string::keyword::name),
          get_arrow_data_type(
              *col->get_as<std::string>(string::keyword::type))));
    }
    schema = arrow::schema(fields);
  }

  std::shared_ptr<arrow::Table> parse(const std::string &path) const override {
    logger->info("parse");

    using namespace pugi;

    xml_document doc;
    const auto result = doc.load_file(path.c_str());

    const auto num_fields = static_cast<size_t>(schema->num_fields());
    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    for (const auto &el : doc) {
      for (size_t i = 0; i < num_fields; ++i) {
        const auto field = schema->field(i);
        const auto type = field->type()->id();
        const auto key = field->name();

        const auto val = el.child(key.c_str()).text();
        if (type == arrow::Int32Type::type_id) {
          std::static_pointer_cast<arrow::Int32Builder>(builders.at(i))
              ->Append(val.as_int());
        } else if (type == arrow::DoubleType::type_id) {
          std::static_pointer_cast<arrow::DoubleBuilder>(builders.at(i))
              ->Append(val.as_double());
        } else if (type == arrow::StringType::type_id) {
          std::static_pointer_cast<arrow::StringBuilder>(builders.at(i))
              ->Append(val.as_string());
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

    throw std::runtime_error("Not implemented");
  }
};

__attribute__((visibility("default"))) extern "C" format_plugin_t
get_plugin(const std::string &id, const config_t &config) {
  return std::make_shared<FormatXmlPlugin>(id, config);
}
} // namespace xml
} // namespace format
} // namespace plugin
} // namespace amanogawa