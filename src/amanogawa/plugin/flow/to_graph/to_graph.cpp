#include "amanogawa/include/api.h"
#include <algorithm>
#include <arrow/api.h>
#include <arrow/compute/api.h>
#include <cmath>
#include <string>
#include <tuple>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
struct FlowToGraphPlugin : FlowPlugin {
  std::string plugin_name() const override { return "to_graph"; }

  explicit FlowToGraphPlugin(const std::string &id, const std::string &from,
                             const config_t &config)
      : FlowPlugin(id, from, config) {
    init_logger();
  }

  // FIXME: Use NN-Descent
  std::shared_ptr<arrow::Table>
  flow(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("flow");

    // TODO: Share input, intermediate and output schema on config
    const auto cols = root_config->get_by_id(from)->get_table_array_qualified(
        "format.csv.columns");
    std::vector<std::shared_ptr<arrow::Field>> fields;
    for (const auto &col : *cols) {
      fields.emplace_back(
          arrow::field(*(col->get_as<std::string>(string::keyword::name)),
                       get_arrow_data_type(
                           *col->get_as<std::string>(string::keyword::type))));
    }

    const auto num_feats = fields.size() - 1; // -1 means label
    const auto num_entries = table->num_rows();

    const auto input_schema = arrow::schema(fields);

    std::vector<std::vector<double>> rows(num_entries);
    for (size_t i = 0; i < num_entries; ++i) {
      std::vector<double> row(num_feats);
      for (size_t j = 0; j < num_feats + 1; ++j) {
        const auto field_name = fields[j]->name();
        if (field_name == "label") {
          continue;
        }

        const auto field_idx = table->schema()->GetFieldIndex(field_name);
        const auto col = table->column(field_idx);
        const auto chunk = col->data()->chunk(0);
        const auto float64_chunk =
            std::static_pointer_cast<arrow::DoubleArray>(chunk);
        row[j - 1] = float64_chunk->Value(i);
      }
      rows[i] = row;
    }

    // TODO: Make configurable
    const auto k = 3;
    const auto p = 1.5;

    std::vector<std::vector<std::tuple<int, double>>> distances(
        num_entries, std::vector<std::tuple<int, double>>(
                         num_entries, std::make_tuple(0, 0)));
    for (size_t i = 0; i < num_entries; ++i) {
      for (size_t j = i + 1; j < num_entries; ++j) {
        double diff = 0;
        for (size_t k = 0; k < num_feats; ++k) {
          diff += std::pow(std::abs(rows[i][k] - rows[j][k]), p);
        }
        const auto distance = std::pow(diff, 1 / p);
        distances[i][j] = std::make_tuple(j, distance);
        distances[j][i] = std::make_tuple(i, distance);
      }
    }

    std::vector<std::shared_ptr<arrow::Field>> output_fields = {
        std::make_shared<arrow::Field>("src", get_arrow_data_type("int")),
        std::make_shared<arrow::Field>("dst", get_arrow_data_type("int"))};
    const auto output_schema = arrow::schema(output_fields);
    arrow::Int32Builder src_builder, dst_builder;

    std::vector<std::tuple<int, int>> tmp; // FIXME
    for (size_t i = 0; i < num_entries; ++i) {
      std::sort(distances[i].begin(), distances[i].end(),
                [](const auto &l, const auto &r) {
                  return std::get<1>(l) < std::get<1>(r);
                });
      for (size_t j = 0; j < k; ++j) {
        const auto min = std::min((int)i, std::get<0>(distances[i][j]));
        const auto max = std::max((int)i, std::get<0>(distances[i][j]));
        tmp.emplace_back(std::make_tuple(min, max));
      }
    }
    std::sort(tmp.begin(), tmp.end(), [](const auto &l, const auto &r) {
      return std::get<1>(l) < std::get<1>(r);
    });
    std::stable_sort(tmp.begin(), tmp.end(), [](const auto &l, const auto &r) {
      return std::get<0>(l) < std::get<0>(r);
    });
    auto prev = std::make_tuple(-1, -1);
    for (const auto &tup : tmp) {
      if (prev != tup) {
        src_builder.Append(std::get<0>(tup));
        dst_builder.Append(std::get<1>(tup));
      }
      prev = tup;
    }

    std::vector<std::shared_ptr<arrow::Column>> columns;
    std::shared_ptr<arrow::Array> src_array, dst_array;
    src_builder.Finish(&src_array);
    dst_builder.Finish(&dst_array);
    columns.emplace_back(
        std::make_shared<arrow::Column>(output_fields[0], src_array));
    columns.emplace_back(
        std::make_shared<arrow::Column>(output_fields[1], dst_array));

    return arrow::Table::Make(output_schema, columns);
  }
};

__attribute__((visibility("default"))) extern "C" flow_plugin_t
get_plugin(const std::string &id, const std::string &from,
           const config_t &config) {
  return std::make_shared<FlowToGraphPlugin>(id, from, config);
}
} // namespace example_add
} // namespace flow
} // namespace plugin
} // namespace amanogawa
