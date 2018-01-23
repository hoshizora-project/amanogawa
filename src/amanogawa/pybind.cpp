#include "amanogawa/core/executor.h"
#include "amanogawa/include/api.h"
#include "amanogawa/include/config_builder.h"
#include <memory>
#include <pybind11/pybind11.h>

namespace amanogawa {
PYBIND11_MODULE(amanogawa, m) {
  namespace py = pybind11;

  m.doc() = "amanogawa: Graph construction meets DAG-based data-oriented "
            "processing engine";

  py::class_<Config, std::shared_ptr<Config>> config(m, "Config");
  config.def("load_from_file", &Config::from_file, "load config from file");

  py::class_<ConfigBuilder, std::shared_ptr<ConfigBuilder>> config_builder(
      m, "ConfigBuilder");
  config_builder.def(py::init<>())
      .def("source", &ConfigBuilder::source, py::return_value_policy::reference)
      .def("flow", &ConfigBuilder::flow, py::return_value_policy::reference)
      .def("branch", &ConfigBuilder::branch, py::return_value_policy::reference)
      .def("confluence", &ConfigBuilder::confluence,
           py::return_value_policy::reference)
      .def("sink", &ConfigBuilder::sink, py::return_value_policy::reference)
      .def("build", &ConfigBuilder::build, py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set<int>, py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set<bool>, py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_bool,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_<int>, py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_<bool>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_bool_,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map_<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map_<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_array_map_<std::string>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map_<int>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map_<double>,
           py::return_value_policy::reference)
      .def("set", &ConfigBuilder::set_map_<std::string>,
           py::return_value_policy::reference);

  m.def("execute", &core::execute, "execute",
        py::return_value_policy::reference);
}
} // namespace amanogawa