#include "amanogawa/core/executor.h"
#include "amanogawa/include/api.h"
#include <memory>
#include <pybind11/pybind11.h>

namespace amanogawa {
PYBIND11_MODULE(amanogawa, m) {
  namespace py = pybind11;

  m.doc() = "amanogawa";
  py::class_<Config, std::shared_ptr<Config>> config(m, "Config");
  config.def("load_from_file", &Config::from_file, "load config from file");
  m.def("execute", &core::execute, "execute",
        py::return_value_policy::reference);
}
} // namespace amanogawa