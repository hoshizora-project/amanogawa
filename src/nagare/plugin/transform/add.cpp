#include <string>
#include <vector>

namespace nagere::plugin::transform {
  extern "C" std::vector<std::string> transform(std::vector<std::string>);
  std::vector<std::string> transform(std::vector<std::string> data) {
    printf("transform is called\n");
    for(auto &row: data) {
      row += "!";
    }
    return data;
  }
}
