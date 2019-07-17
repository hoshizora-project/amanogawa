#include "amanogawa/include/api.h"
#include "measure.h"
#include "nndescent.h"
#include <algorithm>
#include <arrow/api.h>
#include <cmath>
#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace to_graph {
struct FlowToGraphPlugin : FlowPlugin {
  std::string plugin_name() const override { return "to_graph"; }

  explicit FlowToGraphPlugin(const std::string &id, const std::string &from,
                             const config_t &config)
      : FlowPlugin(id, from, config) {
    init_logger();
  }

  // FIXME: Use arrow directly and support discrete table/chunk
  std::shared_ptr<
      std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>
  flow(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("flow");

    // overall
    const auto mode = config->get_as<std::string>("mode").value_or("cosine");

    // knn
    const auto knn_config = config->get_table("knn");
    const auto knn_mode =
        knn_config->get_as<std::string>("mode").value_or("approx");
    const auto k = knn_config->get_as<uint32_t>("k").value_or(3);
    const auto p = knn_config->get_as<double>("p").value_or(1.5);

    // others
    const auto col_from =
        knn_config->get_as<std::size_t>("col_from").value_or(0);
    const auto col_to = knn_config->get_as<std::size_t>("col_to").value_or(
        table->num_columns());

    const auto num_feats = col_to - col_from;
    const auto num_entries = static_cast<std::size_t>(table->num_rows());

    std::unordered_map<size_t, std::unordered_set<size_t>> adj_list{};

    if (mode == "cosine") {
      // FIXME
      std::vector<std::vector<double>> rows(num_entries);
      for (size_t i = 0; i < num_entries; ++i) {
        std::vector<double> row(num_feats);
        for (size_t j = col_from; j < col_to; ++j) {
          const auto col = table->column(j);
          const auto chunk = col->data()->chunk(0); // TMP: only single chunk...
          const auto float64_chunk =
              std::static_pointer_cast<arrow::DoubleArray>(chunk);
          row[j - col_from] = float64_chunk->Value(i);
        }
        rows[i] = row;
      }

      if (knn_mode == "approx") {
        // NN-Descent
        const auto rho = knn_config->get_as<double>("rho").value_or(0.5);
        const auto perturb = knn_config->get_as<double>("perturb").value_or(0);
        const auto num_random_join =
            knn_config->get_as<uint32_t>("num_random_join").value_or(10);
        const auto epoch =
            knn_config->get_as<std::size_t>("epoch").value_or(100);
        const auto delta = knn_config->get_as<double>("delta").value_or(0.001);

        auto nnDescent = NNDescent<std::vector<double>, CosineMeasure>(
            k, CosineMeasure(), rho, perturb, num_random_join);
        nnDescent.exec(rows, epoch, delta);

        for (size_t i = 0; i < num_entries; ++i) {
          adj_list.emplace(i, std::unordered_set<size_t>{}); // FIXME

          const auto nghs = nnDescent.nghs[i];
          for (const auto &ngh : nghs) {
            const auto min = std::min(i, ngh.node_id);
            const auto max = std::max(i, ngh.node_id);
            adj_list[min].emplace(max);
          }
        }
      } else if (knn_mode == "exact") {
        // Naive
        std::vector<std::vector<std::tuple<int, double>>> sims(
            num_entries, std::vector<std::tuple<int, double>>(num_entries));
        const auto cos = CosineMeasure();
        for (size_t i = 0; i < num_entries; ++i) {
          for (size_t j = i + 1; j < num_entries; ++j) {
            const auto sim = cos(rows[i], rows[j]);
            sims[i][j] = std::make_tuple(j, sim);
            sims[j][i] = std::make_tuple(i, sim);
          }
        }

        for (size_t i = 0; i < num_entries; ++i) {
          adj_list.emplace(i, std::unordered_set<size_t>{}); // FIXME

          std::sort(sims[i].begin(), sims[i].end(),
                    [](const auto &l, const auto &r) {
                      return std::get<1>(l) > std::get<1>(r);
                    });
          for (size_t j = 0; j < k; ++j) {
            if (i == j) {
              continue;
            }
            const auto min = std::min((int)i, std::get<0>(sims[i][j]));
            const auto max = std::max((int)i, std::get<0>(sims[i][j]));
            adj_list[min].emplace(max);
          }
        }
      }
    } else if (mode == "bow") {
      const auto col_name = *config->get_as<std::string>("column");
      const auto col_idx = table->schema()->GetFieldIndex(col_name);

      // FIXME
      std::vector<std::string> rows(num_entries);
      const auto col = table->column(col_idx);
      const auto chunk = col->data()->chunk(0); // TMP: only single chunk...
      const auto string_chunk =
          std::static_pointer_cast<arrow::StringArray>(chunk);
      for (size_t i = 0; i < num_entries; ++i) {
        rows[i] = string_chunk->GetString(i);
      }

      if (knn_mode == "approx") {
        // NN-Descent
        const auto rho = knn_config->get_as<double>("rho").value_or(0.5);
        const auto perturb = knn_config->get_as<double>("perturb").value_or(0);
        const auto num_random_join =
            knn_config->get_as<uint32_t>("num_random_join").value_or(10);
        const auto epoch =
            knn_config->get_as<std::size_t>("epoch").value_or(10);
        const auto delta = knn_config->get_as<double>("delta").value_or(0.01);

        // BoW
        std::vector<BoWMeasure::data_t> wakati_maps;
        for (const auto &sentence : rows) {
          const auto words = BoWMeasure::wakati(sentence);
          BoWMeasure::data_t wakati_map{};
          for (const auto &word : words) {
            wakati_map[word]++;
          }
          wakati_maps.emplace_back(wakati_map);
        }

        auto nnDescent = NNDescent<BoWMeasure::data_t, BoWMeasure>(
            k, BoWMeasure(p), rho, perturb, num_random_join);
        nnDescent.exec(wakati_maps, epoch, delta);

        for (size_t i = 0; i < num_entries; ++i) {
          adj_list.emplace(i, std::unordered_set<size_t>{}); // FIXME

          const auto nghs = nnDescent.nghs[i];
          for (const auto &ngh : nghs) {
            const auto min = std::min(i, ngh.node_id);
            const auto max = std::max(i, ngh.node_id);
            adj_list[min].emplace(max);
          }
        }
      } else if (knn_mode == "exact") {
        throw std::runtime_error("Not implemented: BoW > exact");
      }
    } else {
      throw std::runtime_error("Not implemented: " + mode + " > " + knn_mode);
    }

    std::vector<std::shared_ptr<arrow::Field>> output_fields = {
        std::make_shared<arrow::Field>("src", get_arrow_data_type("int")),
        std::make_shared<arrow::Field>("dst", get_arrow_data_type("int"))};
    const auto output_schema = arrow::schema(output_fields);
    arrow::Int32Builder src_builder, dst_builder;

    for (size_t i = 0; i < num_entries; ++i) {
      const auto nghs = adj_list.at(i);
      for (const auto &ngh : nghs) {
        src_builder.Append(i);
        dst_builder.Append(ngh);
      }
    }

    std::vector<std::shared_ptr<arrow::Column>> columns;
    std::shared_ptr<arrow::Array> src_array, dst_array;
    src_builder.Finish(&src_array);
    dst_builder.Finish(&dst_array);
    columns.emplace_back(
        std::make_shared<arrow::Column>(output_fields[0], src_array));
    columns.emplace_back(
        std::make_shared<arrow::Column>(output_fields[1], dst_array));

    const auto tos = config->get_table_array(string::keyword::to);
    auto result = std::make_shared<
        std::unordered_map<std::string, std::shared_ptr<arrow::Table>>>();

    if (!tos) {
      result->emplace(id, arrow::Table::Make(output_schema, columns));
    } else {
      auto it = tos->begin();
      result->emplace(*(*it)->get_as<std::string>(string::keyword::name),
                      arrow::Table::Make(output_schema, columns));

      it++;
      const auto cols =
          (*it)->get_array_of<std::string>(string::keyword::columns);
      std::vector<std::shared_ptr<arrow::Field>> meta_fields;
      std::vector<std::shared_ptr<arrow::Column>> meta_columns;
      for (const auto &col : *cols) {
        meta_fields.emplace_back(table->schema()->GetFieldByName(col));
        meta_columns.emplace_back(
            table->column(table->schema()->GetFieldIndex(col)));
      }
      result->emplace(
          *(*it)->get_as<std::string>(string::keyword::name),
          arrow::Table::Make(arrow::schema(meta_fields), meta_columns));
    }

    return result;
  }
};

extern "C" flow_plugin_t get_plugin(const std::string &id,
                                    const std::string &from,
                                    const config_t &config) {
  return std::make_shared<FlowToGraphPlugin>(id, from, config);
}
} // namespace to_graph
} // namespace flow
} // namespace plugin
} // namespace amanogawa
