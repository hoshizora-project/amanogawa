#include <string>
#include <vector>
#include "amanogawa/plugin/source/file/csv.h"

namespace amanogawa {
namespace plugin {
namespace source {
namespace file {
extern "C" std::vector<std::string> spring(const std::string &file_name) {
  printf("source is called\n");
  io::CSVReader<2> in(file_name);
  in.read_header(io::ignore_extra_column, "id", "name");
  uint32_t id;
  std::string name;
  std::vector<std::string> result;
  while (in.read_row(id, name)) {
    result.emplace_back(std::to_string(id) + "|" + name);
  }

  return result;
}
}
}
}
}