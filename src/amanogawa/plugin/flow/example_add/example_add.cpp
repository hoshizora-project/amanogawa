#include <string>
#include <vector>

namespace amanogawa {
namespace plugin {
namespace flow {
namespace example_add {
extern "C" std::vector<std::string> flow(std::vector<std::string> &data) {
  printf("flow is called\n");
  for (auto &row: data) {
    row += "!";
  }
  return data;
}
}
}
}
}
