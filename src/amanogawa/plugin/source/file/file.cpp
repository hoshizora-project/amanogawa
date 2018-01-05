#include "amanogawa/core/confing.h"
#include "amanogawa/include/source_plugin.h"
#include "text-csv/include/text/csv/istream.hpp"
#include <arrow/api.h>
#include <functional>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {

static std::unordered_map<std::string, std::string> normalize_table = {
    {"int32", "int32"},
    {"int", "int32"},
    {"float64", "float64"},
    {"double", "float64"},
    {"utf8", "utf8"},
    {"string", "utf8"}};

static std::unordered_map<std::string,
                          std::function<std::shared_ptr<arrow::DataType>(void)>>
    arrow_data_type_table = {
        {"int32", []() { return arrow::int32(); }},
        {"float64", []() { return arrow::float64(); }},
        {"utf8", []() { return arrow::utf8(); }}};

auto get_arrow_data_type(const std::string &type) {
  return arrow_data_type_table.at(normalize_table.at(type))();
}

static std::unordered_map<
    std::string,
    std::function<std::shared_ptr<arrow::ArrayBuilder>(arrow::MemoryPool *)>>
    arrow_builder_table = {
        {"int32",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::Int32Builder>(pool);
         }},
        {"float64",
         [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::DoubleBuilder>(pool);
         }},
        {"utf8", [](arrow::MemoryPool *pool) {
           return std::make_shared<arrow::StringBuilder>(pool);
         }}};

auto get_arrow_builder(const std::string &type,
                       arrow::MemoryPool *pool = arrow::default_memory_pool()) {
  return arrow_builder_table.at(normalize_table.at(type))(pool);
}

struct SourceFilePlugin : SourcePlugin {
  const logger_t logger = get_logger(plugin_full_name());

  std::string plugin_name() const override { return "file"; }

  const core::Config entire_config;
  const core::Config::config_map config;
  std::shared_ptr<arrow::Schema> schema;

  // TODO: Validate config
  explicit SourceFilePlugin(const core::Config &config)
      : entire_config(config),
        config(entire_config.source->get_table(plugin_name())) {
    const auto cols =
        config.source->get_table_array_qualified("format.csv.columns");

    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col : *cols) {
      fields.emplace_back(std::make_shared<arrow::Field>(
          *(col->get_as<std::string>("name")),
          get_arrow_data_type(*col->get_as<std::string>("type"))));
    }
    schema = arrow::schema(std::move(fields));
  }

  // TODO: Want to use https://issues.apache.org/jira/browse/ARROW-25
  std::shared_ptr<arrow::Table> spring() const override {
    logger->info("spring");

    const auto file_name = *config->get_as<std::string>("path");
    std::ifstream fs(file_name);
    text::csv::csv_istream csvs(fs);

    const auto num_fields = static_cast<size_t>(schema->num_fields());

    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    // FIXME: Manage invalid row
    while (csvs) {
      for (size_t i = 0; i < num_fields; ++i) {
        const auto type = schema->field(static_cast<int>(i))->type()->name();
        // FIXME: Use visitor or pre-planing (concat field info for each row)
        if (type == "int32") {
          int val;
          csvs >> val;
          std::dynamic_pointer_cast<arrow::Int32Builder>(builders.at(i))
              ->Append(val);
        } else if (type == "float64") {
          double val;
          csvs >> val;
          std::dynamic_pointer_cast<arrow::DoubleBuilder>(builders.at(i))
              ->Append(val);
        } else if (type == "utf8") {
          std::string val;
          csvs >> val;
          std::dynamic_pointer_cast<arrow::StringBuilder>(builders.at(i))
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

extern "C" get_source_plugin_return_t
get_source_plugin(const core::Config &config) {
  return std::make_unique<SourceFilePlugin>(config);
}
} // namespace file
} // namespace source
} // namespace plugin
} // namespace amanogawa