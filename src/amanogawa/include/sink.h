#ifndef AMANOGAWA_SINK_H
#define AMANOGAWA_SINK_H

#include <string>
#include <vector>

namespace amanogawa {
void drain(const std::string &file_name, const std::vector<std::string> &data);
}

#endif