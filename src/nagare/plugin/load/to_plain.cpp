#include <string>
#include <vector>
#include <fstream>

namespace nagere::plugin::load {
  extern "C" void load(const std::string &, const std::vector<std::string> &);
  void load(const std::string &file_name, const std::vector<std::string> &data) {
    printf("load is called\n");
    std::ofstream output(file_name);
    for(const auto &row: data) {
      output << row;
    }
    output.close();
  }
}
