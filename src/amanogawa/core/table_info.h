#ifndef AMANOGAWA_TABLE_INFO_H
#define AMANOGAWA_TABLE_INFO_H

#include "amanogawa/core/column_info.h"
#include <vector>

namespace amanogawa {
namespace core {
struct TableInfo {
  // std::string name;
  ColumnsInfo columns_info;
};
} // namespace core
} // namespace amanogawa

#endif // AMANOGAWA_TABLE_INFO_H
