#include <db.h>
#include <string>
#include <meta.h>
#include "sha256.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <image.h>
#include <sstream> 
#include <fsutil.h>

#ifdef _WIN32
  #include <process.h>
  #define getpid _getpid
#else
  #include <unistd.h>
#endif

std::string generate_id() {
  using namespace std::chrono;

  static std::atomic<uint64_t> counter{0};

  auto now = system_clock::now();
  auto now_s = time_point_cast<seconds>(now);
  std::time_t t = system_clock::to_time_t(now_s);

  // Format timestamp as YYYYMMDD-HHMMSS
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y%m%d-%H%M%S")
      << "-" << getpid()
      << "-" << std::setw(6) << std::setfill('0') << counter++;

  return oss.str();
}

ImageDB ImageDB::Open(const std::string& db_path) {
    namespace fs = std::filesystem;

    if(db_path.empty()) {
        throw std::invalid_argument("Open: db path cannot be empty");
    }

    fs::path root(db_path);

    if(fs::exists(root) && fs::is_regular_file(root)) {
        throw std::runtime_error("Open: path exists but is file: " + root.string());
    }

    if(!fs::exists(db_path)) {
        std::error_code ec;

        if(!fs::create_directories(root, ec) || ec) {
            throw std::runtime_error("Open: failed to create db_root: " + root.string());
        }
    } else if(!fs::is_directory(root)){
        throw std::runtime_error("Open path is not a direcoty: " + root.string());
    }

    fs::path abs = fs::weakly_canonical(root);

    ImageDB db = ImageDB();
    db.db_root = abs.string();
    db.manifest_path      = (abs / "MANIFEST").string();
    db.wal_path           = (abs / "WAL.current").string();
    db.catalog_dir        = (abs / "catalog").string();
    db.catalog_meta_path  = (abs / "catalog" / "meta.ndjson").string();
    db.blobs_dir          = (abs / "blobs").string();
    db.thumbs_dir         = (abs / "thumbs").string();

    if(fs::exists(db.manifest_path)){
        std::ifstream in(db.manifest_path);

        if(!in.good()) {
            throw std::runtime_error("Open: cannot read MANIFEST at " + db.manifest_path);
        }
        std::string line;
        if(!std::getline(in, line)){
            throw std::runtime_error("Open: MANIFEST is empty at " + db.manifest_path);
        }

        if(!line.empty() && line.back() == '\r') line.pop_back();

        fs::path manifest_target = abs / line;

        if(!fs::exists(manifest_target)){
            throw std::runtime_error("Open: Manifest points to missing file: " + manifest_target.string());
        }

        db.is_initialized = true;
    } else {
        db.is_initialized = false;
    }

    return db;
}

bool ImageDB::Init(){
    namespace fs = std::filesystem;

    // 1) Sanity: root must be a directory (create if missing)
    if (db_root.empty()) {
        std::cerr << "Init: db_root is empty\n";
        return false;
    }

    if (fs::exists(db_root) && !fs::is_directory(db_root)) {
        std::cerr << "Init: path exists but is not a directory: " << db_root << "\n";
        return false;
    }

    if (!fs::exists(db_root)) {
        std::error_code ec;
        if (!fs::create_directories(db_root, ec) || ec) {
            std::cerr << "Init: failed to create db_root: " << db_root << "\n";
            return false;
        }
    }

    if(this->is_initialized && fs::exists(manifest_path)) {
        std::cout<<"Database is already initialized. Skipping...\n";
        return true;
    }

    // 2) Ensure directory structure (idempotent)
    std::error_code ec;
    fs::create_directories(catalog_dir, ec);
    fs::create_directories(blobs_dir, ec);
    fs::create_directories(thumbs_dir, ec);

    // 3) Ensure catalog/meta.ndjson exists BEFORE publishing MANIFEST
    const fs::path meta_abs = fs::path(catalog_dir) / "meta.ndjson";
    if (!fs::exists(meta_abs)) {
        std::ofstream meta_out(meta_abs, std::ios::binary);
        if (!meta_out) {
            std::cerr << "Init: failed to create " << meta_abs.string() << "\n";
            return false;
        }
        meta_out.close();
    }

    // 4) Publish MANIFEST atomically (write temp, then rename)
    //    MANIFEST must contain a RELATIVE path inside db_root.
    const std::string manifest_target_rel = "catalog/meta.ndjson";
    if (!fs::exists(manifest_path)) {
        fs::path tmp = fs::path(db_root) / ("MANIFEST.tmp." + std::to_string(::getpid()));
        {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "Init: cannot create temp MANIFEST: " << tmp.string() << "\n";
            return false;
        }
        out << manifest_target_rel << "\n";
        out.close();
        }
        std::error_code rn_ec;
        fs::rename(tmp, manifest_path, rn_ec);
        if (rn_ec) {
            std::cerr << "Init: atomic rename to MANIFEST failed: " << rn_ec.message() << "\n";
            return false;
        }
    } else {
        // Validate existing MANIFEST points to expected location
        std::ifstream in(manifest_path);
        if (!in) {
        std::cerr << "Init: cannot read existing MANIFEST\n";
        return false;
        }
        std::string line;
        if (!std::getline(in, line)) {
        std::cerr << "Init: existing MANIFEST is empty\n";
        return false;
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line != manifest_target_rel) {
        std::cerr << "Init: MANIFEST points to unexpected target: " << line
                    << " (expected " << manifest_target_rel << ")\n";
        return false;
        }
    }

    // 5) Ensure WAL.current exists (do NOT truncate if present)
    if (!fs::exists(wal_path)) {
        std::ofstream wal(wal_path, std::ios::binary | std::ios::trunc);
        if (!wal) {
        std::cerr << "Init: failed to create WAL.current\n";
        return false;
        }
        wal.close();
    }

    // 6) Mark initialized
    is_initialized = true;
    std::cout << "Initialized DB at " << db_root << "\n";
    return true;
}

bool ImageDB::ImportFile(const std::string& file){
    namespace fs = std::filesystem;

    std::string hash = sha256_file(file);
    std::string blob_dir = db_root + "/blobs" + hash.substr(0,2);
    std::string blob_path = blob_dir + "/" + hash;
    std::string thumbs_path = db_root + "/thumbs";

    if(fs::exists(blob_path)) {
        std::cout<<"Already present (sha256 match). Skipped.\n";
        return false;
    }

    atomic_copy(file, blob_path);

    ImgDims dims;

    if(!read_dims(file, &dims)){
        std::cerr<<"Failed to read image dimensions\n";
        return false;
    }

    ImageMeta m;
    m.image_id = generate_id();
    m.sha256 = hash;
    m.mime = "image/jpeg";
    m.width = dims.width;
    m.height = dims.height;
    m.bytes = std::filesystem::file_size(file);
    m.created_unix = std::time(nullptr);

    append_json_line(catalog_dir + "/meta.ndjson", meta_to_json(m));
    make_thumbnail_256(file, thumbs_path + "/" + m.image_id + "_256.jpg");

    std::cout << "Imported: " << m.image_id << " sha256=" << hash << "\n";
    return true;
};