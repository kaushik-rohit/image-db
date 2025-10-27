#pragma once
#include <string>

bool ensure_dirs(const std::string& path);
bool atomic_copy(const std::string& src, const std::string& dst);