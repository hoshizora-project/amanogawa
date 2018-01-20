#include "amanogawa/include/confluence_plugin.h"
#include <arrow/api.h>

namespace amanogawa {
namespace plugin {
namespace confluence {
namespace key {
struct ConfluenceKeyPlugin : ConfluencePlugin {
  std::string plugin_name() const override { return "key"; }

  ConfluenceKeyPlugin(const std::string &id, const std::string &from_left,
                      const std::string &from_right, const config_t &config)
      : ConfluencePlugin(id, from_left, from_right, config) {
    init_logger();
  }

  std::shared_ptr<arrow::Table>
  int32_join(const std::shared_ptr<arrow::Table> &left_table,
             const std::shared_ptr<arrow::Table> &right_table,
             const std::shared_ptr<arrow::Column> &left_col,
             const std::shared_ptr<arrow::Column> &right_col) const {
    // Use left key as a name of joined column
    std::vector<std::shared_ptr<arrow::Field>> output_fields;
    for (const auto &field : left_table->schema()->fields()) {
      output_fields.emplace_back(field);
    }
    for (const auto &field : right_table->schema()->fields()) {
      if (field->name() != right_col->name()) {
        output_fields.emplace_back(field);
      }
    }

    const auto num_fields = output_fields.size();
    std::shared_ptr<arrow::Schema> output_schema = arrow::schema(output_fields);
    std::vector<std::shared_ptr<arrow::ArrayBuilder>> builders;
    builders.reserve(num_fields);
    for (const auto &field : output_schema->fields()) {
      builders.emplace_back(get_arrow_builder(field->type()->name()));
    }

    // TMP: Require the type of key is int32
    for (size_t i = 0, l_num_chunk = left_col->data()->num_chunks();
         i < l_num_chunk; ++i) {
      const auto l_chunk = left_col->data()->chunk(i);
      for (size_t ii = 0, l_end = l_chunk->length(); ii < l_end; ++ii) {
        const auto l_int32_chunk =
            std::static_pointer_cast<arrow::Int32Array>(l_chunk);
        for (size_t j = 0, r_num_chunk = right_col->data()->num_chunks();
             j < r_num_chunk; ++j) {
          const auto r_chunk = right_col->data()->chunk(j);
          const auto r_int32_chunk =
              std::static_pointer_cast<arrow::Int32Array>(r_chunk);
          for (size_t jj = 0, r_end = r_chunk->length(); jj < r_end; ++jj) {
            if (l_int32_chunk->Value(ii) == r_int32_chunk->Value(jj)) {
              // generate row
              for (size_t l = 0; l < num_fields; ++l) {
                const auto field = output_schema->field(l);
                const auto type = field->type()->id();
                const auto contain_in_left =
                    left_table->schema()->GetFieldIndex(field->name()) >= 0;
                const auto kk = contain_in_left ? ii : jj;
                const auto chunk =
                    contain_in_left
                        ? left_table
                              ->column((int)left_table->schema()->GetFieldIndex(
                                  field->name()))
                              ->data()
                              ->chunk(i)
                        : right_table
                              ->column(
                                  (int)right_table->schema()->GetFieldIndex(
                                      field->name()))
                              ->data()
                              ->chunk(j);

                if (type == arrow::Int32Type::type_id) {
                  const auto val =
                      std::static_pointer_cast<arrow::Int32Array>(chunk)->Value(
                          kk);
                  std::static_pointer_cast<arrow::Int32Builder>(builders.at(l))
                      ->Append(val);
                } else if (type == arrow::DoubleType::type_id) {
                  const auto val =
                      std::static_pointer_cast<arrow::DoubleArray>(chunk)
                          ->Value(kk);
                  std::static_pointer_cast<arrow::DoubleBuilder>(builders.at(l))
                      ->Append(val);
                } else if (type == arrow::StringType::type_id) {
                  const auto val =
                      std::static_pointer_cast<arrow::StringArray>(chunk)
                          ->GetString(kk);
                  std::static_pointer_cast<arrow::StringBuilder>(builders.at(l))
                      ->Append(val);
                } else {
                  logger->warn("Detected unsupported type: {}", type);
                }
              }
            }
          }
        }
      }
    }

    std::vector<std::shared_ptr<arrow::Column>> columns;
    columns.reserve(num_fields);
    for (size_t i = 0; i < num_fields; ++i) {
      std::shared_ptr<arrow::Array> array;
      builders.at(i)->Finish(&array);
      columns.emplace_back(
          std::make_shared<arrow::Column>(output_schema->field(i), array));
    }

    return arrow::Table::Make(output_schema, columns);
  }

  std::shared_ptr<arrow::Table>
  join(const std::shared_ptr<arrow::Table> &left_table,
       const std::shared_ptr<arrow::Table> &right_table) const override {
    logger->info("join");

    const auto froms = config->get_table_array(string::keyword::from);
    // FIXME
    const auto left = *(*froms).begin();
    const auto right = *((*froms).begin() + 1);
    const auto left_key = *left->get_as<std::string>("key");
    const auto right_key = *right->get_as<std::string>("key");
    const auto left_col =
        left_table->column(left_table->schema()->GetFieldIndex(left_key));
    const auto right_col =
        right_table->column(right_table->schema()->GetFieldIndex(right_key));

    // let left be smaller
    return int32_join(left_table, right_table, left_col, right_col);
  }
}; // namespace key
__attribute__((visibility("default"))) extern "C" confluence_plugin_t
get_plugin(const std::string &id, const std::string &from_left,
           const std::string &from_right, const config_t &config) {
  return std::make_shared<ConfluenceKeyPlugin>(id, from_left, from_right,
                                               config);
}
} // namespace key
} // namespace confluence
} // namespace plugin
} // namespace amanogawa
