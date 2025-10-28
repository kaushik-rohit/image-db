#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

enum class WalKind : uint8_t {Pending, Ok, Error, Checkpoint };
enum class WalOp: uint8_t { Import };

// Actual transition state record
struct WalRecord {
    WalKind       kind;
    WalOp         op;
    int64_t       ts_unix;
    std::string   sha256;
    std::string   image_id;
    std::string   src_path;
    std::string   reason;
};

// State while scaning the WAL for recovery
struct WalPending {
    int64_t       ts_unix = 0;
    std::string   sha256;
    std::string   image_id;
    std::string   src_path;

    bool          has_ok = false;
};


struct WalScanResult {
    std::vector<WalRecord> records;
    std::vector<WalPending> pendings;
    size_t truncated_bytes = 0;
};

enum class FsyncPolicy : uint8_t {
    Always,
    Grouped,
    None
};



class Wal {
public:
    static std::optional<Wal> Open(const std::string& path, FsyncPolicy policy = FsyncPolicy::Always);

  bool AppendPending(WalOp op, const std::string& sha256,
                     const std::string& image_id, const std::string& src_path, int64_t ts_unix);
  bool AppendOk(const std::string& sha256, const std::string& image_id, int64_t ts_unix);
  bool AppendError(const std::string& sha256, const std::string& image_id,
                   const std::string& reason, int64_t ts_unix);

    bool Flush();

    WalScanResult Scan() const;

    const std::string& last_error() const;

    // Not thread-safe. One writer, many readers by convention.
    Wal(const Wal&) = delete; Wal& operator=(const Wal&) = delete;
    Wal(Wal&&) noexcept = default; Wal& operator=(Wal&&) noexcept = default;

private:
  explicit Wal(struct Impl* p); // pimpl
  Impl* impl_;
};

