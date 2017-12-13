#ifndef AMANOGAWA_COLUMN_H
#define AMANOGAWA_COLUMN_H

#include <string>

namespace amanogawa {
namespace core {
struct Column {
  std::string name;
  uint32_t index;
};
}
}

#endif