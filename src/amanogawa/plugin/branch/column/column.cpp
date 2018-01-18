#ifndef AMANOGAWA_COLUMN_CPP
#define AMANOGAWA_COLUMN_CPP

#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace branch {
namespace column {
struct BranchColumnPlugin : BranchPlugin {
  std::string plugin_name() const override { return "column"; }
  const logger_t logger = get_logger(BranchPlugin::plugin_full_name());

  explicit BranchColumnPlugin(const std::string &id, const std::string &from,
                              const config_t &config)
      : BranchPlugin(id, from, config) {}

  std::shared_ptr<
      std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>
  branch(const std::shared_ptr<arrow::Table> &table) const override {
    const auto branches = config->get_table_array(string::keyword::to);
    auto tables = std::make_shared<
        std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>();
    for (const auto &branch : *branches) {
      std::vector<std::shared_ptr<arrow::Field>> fields;
      std::vector<std::shared_ptr<arrow::Column>> columns;
      const auto col_names = *branch->get_array_of<std::string>("columns");
      for (const auto &col_name : col_names) {
        fields.emplace_back(table->schema()->GetFieldByName(col_name));
        columns.emplace_back(
            table->column(table->schema()->GetFieldIndex(col_name)));
      }
      const auto schema = arrow::schema(fields);
      tables->emplace(*branch->get_as<std::string>("name"),
                      arrow::Table::Make(schema, columns));
    }

    return tables;
  }
};

__attribute__((visibility("default"))) extern "C" branch_plugin_t
get_plugin(const std::string &id, const std::string &from,
           const config_t &config) {
  return std::make_shared<BranchColumnPlugin>(id, from, config);
}
} // namespace column
} // namespace branch
} // namespace plugin
} // namespace amanogawa

#endif // AMANOGAWA_COLUMN_CPP
