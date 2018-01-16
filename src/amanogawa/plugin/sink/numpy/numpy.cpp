#include "amanogawa/include/api.h"
#include <arrow/api.h>
#include <arrow/python/api.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>

namespace amanogawa {
namespace plugin {
namespace sink {
namespace numpy {
struct SinkNumpyPlugin : SinkPlugin {
  std::string plugin_name() const override { return "numpy"; }
  const logger_t logger = get_logger(SinkPlugin::plugin_full_name());

  explicit SinkNumpyPlugin(const std::string &id, const std::string &from,
                           const config_t &config)
      : SinkPlugin(id, from, config) {}

  void *drain(const std::shared_ptr<arrow::Table> &table) const override {
    logger->info("drain");

    std::vector<int64_t> indices;
    const auto cols = config->get_array_of<std::string>("columns");
    for (const auto &col : *cols) {
      indices.emplace_back(table->schema()->GetFieldIndex(col));
    }

    auto x = table->column(0);
    auto y = x->data();
    auto z = y->chunk(0);
    auto u = z->data();
    auto w = u->buffers;
    auto h = w[0];

    arrow::Tensor tensor(arrow::float64(), h, {1, table->num_rows()});

    Py_Initialize();
    // PyObject *t;
    // t = PyTuple_New(3);
    // PyTuple_SetItem(t, 0, PyLong_FromLong(1L));
    // PyTuple_SetItem(t, 1, PyLong_FromLong(2L));
    // PyTuple_SetItem(t, 2, PyUnicode_FromString("three"));

    auto base = new PyObject();
    auto res = new PyObject();
    arrow::py::TensorToNdarray(tensor, base, &res);
    return res;
  }
};

__attribute__((visibility("default"))) extern "C" get_sink_plugin_return_t
get_sink_plugin(const std::string &id, const std::string &from,
                const config_t &config) {
  return std::make_unique<SinkNumpyPlugin>(id, from, config);
}
} // namespace numpy
} // namespace sink
} // namespace plugin
} // namespace amanogawa
