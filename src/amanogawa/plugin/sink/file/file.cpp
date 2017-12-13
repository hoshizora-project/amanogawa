#include <string>
#include <vector>
#include <fstream>
#include "amanogawa/include/sink.h"

namespace amanogawa {
namespace plugin {
namespace sink {
namespace file {
extern "C" void drain(const std::string &file_name, const std::vector<std::string> &data) {
  printf("sink is called\n");
  std::ofstream output(file_name);
  for (const auto &row: data) {
    output << row;
  }
  output.close();
}
}
}
}
}