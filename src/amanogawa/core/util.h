#ifndef AMANOGAWA_UTIL_H
#define AMANOGAWA_UTIL_H

#include "spdlog/spdlog.h"

namespace amanogawa {
namespace core {
using logger_t = std::shared_ptr<spdlog::logger>;
static logger_t get_logger(const std::string &id) {
  const auto fqid = "amanogawa@" + id;
  auto logger = spdlog::get(fqid);
  return logger != nullptr ? logger : spdlog::stderr_color_mt(fqid);
}
} // namespace core
} // namespace amanogawa

#endif // AMANOGAWA_UTIL_H
