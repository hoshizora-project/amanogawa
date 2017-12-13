#ifndef AMANOGAWA_COMMON_H
#define AMANOGAWA_COMMON_H

namespace amanogawa {
constexpr auto source_spring = "spring";
constexpr auto format_parse = "parse";
constexpr auto flow_flow = "flow";
constexpr auto format_format = "format";
constexpr auto sink_drain = "drain";

namespace func_name {
constexpr auto get_source_plugin = "get_source_plugin";
constexpr auto get_flow_plugin = "get_flow_plugin";
constexpr auto get_sink_plugin = "get_sink_plugin";
} // namespace func_name
} // namespace amanogawa

#endif // AMANOGAWA_COMMON_H
