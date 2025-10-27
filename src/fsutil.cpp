#include "fsutil.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <system_error>
#include <iostream>

#ifdef _WIN32
  #include <process.h>
  #define getpid _getpid
#else
  #include <unistd.h>
#endif

bool ensure_dirs(const std::string& path) {
  namespace fs = std::filesystem;
  std::error_code ec;
  if (fs::exists(path, ec)) {
    if (fs::is_directory(path, ec)) return true;
    std::cerr << "ensure_dirs: path exists but is not a directory: " << path << "\n";
    return false;
  }
  if (!fs::create_directories(path, ec) || ec) {
    std::cerr << "ensure_dirs: failed to create " << path << ": " << ec.message() << "\n";
    return false;
  }
  return true;
}

bool atomic_copy(const std::string& src, const std::string& dst) {
  namespace fs = std::filesystem;
  std::error_code ec;

  fs::path dst_path(dst);
  fs::path dir = dst_path.parent_path();
  if (!dir.empty() && !fs::exists(dir)) {
    if (!fs::create_directories(dir, ec) || ec) {
      std::cerr << "atomic_copy: failed to create parent dir " << dir << ": " << ec.message() << "\n";
      return false;
    }
  }

  // temp file in the SAME directory as dst so rename is atomic
  std::mt19937_64 rng(static_cast<uint64_t>(getpid()) ^ 0x9e3779b97f4a7c15ULL);
  uint64_t r = rng();
  fs::path tmp = dir / (dst_path.filename().string() + ".tmp." + std::to_string(r));

  // copy without overwrite
  fs::copy_file(src, tmp, fs::copy_options::none, ec);
  if (ec) {
    std::cerr << "atomic_copy: copy_file failed: " << ec.message() << "\n";
    // best-effort cleanup
    std::error_code ignore;
    fs::remove(tmp, ignore);
    return false;
  }

  // atomic publish
  fs::rename(tmp, dst_path, ec);
  if (ec) {
    std::cerr << "atomic_copy: rename failed: " << ec.message() << "\n";
    std::error_code ignore;
    fs::remove(tmp, ignore);
    return false;
  }
  return true;
}

bool append_json_line(){
    
}