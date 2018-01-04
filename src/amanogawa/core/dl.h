#ifndef AMANOGAWA_DL_H
#define AMANOGAWA_DL_H

#include <dlfcn.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

namespace amanogawa {
namespace core {
class DL {
public:
#ifdef __APPLE__
  static constexpr auto default_mode = RTLD_LOCAL; // mac
  static constexpr auto ext = "dylib";
#elif __linux__
  static constexpr auto default_mode = RTLD_LAZY; // linux
  static constexpr auto ext = "so";
#endif

  ~DL() {
    if (handle != nullptr) {
      ::dlclose(handle);
    }
  }

  static std::shared_ptr<DL> open(const std::string &lib_name,
                                  const int mode = default_mode) {
    auto handle = ::dlopen(("lib" + lib_name + "." + ext).c_str(), mode);
    if (handle != nullptr) {
      return std::shared_ptr<DL>(new DL(handle));
    } else {
      throw std::runtime_error("Failed to open " + lib_name + ". " +
                               ::dlerror());
    }
  }

  template <class T> T sym(const std::string &func_name) {
    auto func = ::dlsym(this->handle, func_name.c_str());
    if (func != nullptr) {
      return reinterpret_cast<T>(func);
    } else {
      throw std::runtime_error("Failed to load function " + func_name + ". " +
                               ::dlerror());
    }
  }

private:
  void *const handle;
  explicit DL(void *handle) : handle(handle) {}
};
} // namespace core
} // namespace amanogawa

#endif // AMANOGAWA_DL_H
