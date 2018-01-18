#ifndef AMANOGAWA_COMMON_H
#define AMANOGAWA_COMMON_H

namespace amanogawa {
namespace string {
namespace clazz {
constexpr auto _source = "source";
namespace source {
constexpr auto spring = "spring";
}
constexpr auto _flow = "flow";
namespace flow {
constexpr auto flow = "flow";
}
constexpr auto _branch = "branch";
namespace branch {
constexpr auto branch = "branch";
}
constexpr auto _confluence = "confluence";
namespace confluence {
constexpr auto confluent = "confluent";
}
constexpr auto _sink = "sink";
namespace sink {
constexpr auto drain = "drain";
}
} // namespace clazz

namespace func {
constexpr auto get_plugin = "get_plugin";
} // namespace func

namespace keyword {
constexpr auto clazz = "clazz";
constexpr auto type = "type";
constexpr auto from = "from";
constexpr auto to = "to";
} // namespace keyword
} // namespace string
} // namespace amanogawa

#endif // AMANOGAWA_COMMON_H
