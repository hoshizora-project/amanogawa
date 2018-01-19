#ifndef AMANOGAWA_COMMON_H
#define AMANOGAWA_COMMON_H

namespace amanogawa {
namespace string {
namespace clazz {
constexpr auto source = "source";
constexpr auto flow = "flow";
constexpr auto branch = "branch";
constexpr auto confluence = "confluence";
constexpr auto sink = "sink";
} // namespace clazz

namespace func {
constexpr auto get_plugin = "get_plugin";
} // namespace func

namespace keyword {
constexpr auto clazz = "clazz";
constexpr auto type = "type";
constexpr auto from = "from";
constexpr auto to = "to";
constexpr auto name = "name";
constexpr auto columns = "columns";
} // namespace keyword
} // namespace string
} // namespace amanogawa

#endif // AMANOGAWA_COMMON_H
