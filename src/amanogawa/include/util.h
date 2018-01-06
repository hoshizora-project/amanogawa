#ifndef AMANOGAWA_UTIL_H
#define AMANOGAWA_UTIL_H

#include "spdlog/spdlog.h"

namespace amanogawa {
using logger_t = std::shared_ptr<spdlog::logger>;
// TODO: Thread-safe
static logger_t get_logger(const std::string &id) {
  const auto fqid = "amanogawa@" + id;
  auto logger = spdlog::get(fqid);
  return logger != nullptr ? logger : spdlog::stderr_color_mt(fqid);
}
} // namespace amanogawa

#endif // AMANOGAWA_UTIL_H
