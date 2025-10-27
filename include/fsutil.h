#pragma once
#include <string>

bool ensure_dirs(const std::string& path);
bool atomic_copy(const std::string& src, const std::string& dst);
bool append_json_line(const std::string& path, const std::string& json);