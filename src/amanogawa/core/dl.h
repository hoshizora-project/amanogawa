#ifndef AMANOGAWA_DL_H
#define AMANOGAWA_DL_H

#include <dlfcn.h>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

#define STRINGIFY(str) #str
#define STRING_OF(str) STRINGIFY(str)

namespace amanogawa {
namespace core {
class DL {
public:
#ifdef __APPLE__
  static constexpr auto default_mode = RTLD_LOCAL; // mac
#elif __linux__
  static constexpr auto default_mode = RTLD_LAZY; // linux
#endif

  ~DL() {
    if (handle != nullptr) {
      ::dlclose(handle);
    }
  }

  static std::shared_ptr<DL> open(const std::string &lib_name,
                                  const bool is_full_lib_name = false,
                                  const int mode = default_mode) {
    const auto full_lib_name =
        is_full_lib_name ? lib_name
                         : "lib" + lib_name + "." + STRING_OF(LIBS_EXT);

    const auto handle = ::dlopen(full_lib_name.c_str(), mode);
    if (handle != nullptr) {
      return std::shared_ptr<DL>(new DL(handle));
    } else {
      throw std::runtime_error("Failed to open " + full_lib_name + ". " +
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
