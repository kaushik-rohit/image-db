#include <db.h>
#include <string>
#include <meta.h>
#include "sha.h"
#include<filesystem>

void ensure_dirs(const std::string& filepath) {
    if(std::filesystem::exists(filepath)) {
        cout<<"Filepath already exists\n";
        return;
    }

    std::filesystem::create_directoy(filepath);
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
    } else if(!fs::is_directoy(root)){
        throw std::runtime_error("Open path is not a direcoty: " + root.string());
    }

    fs::path abs = fs::weakly_canonical(root);

    ImageDB db = ImageDB();
    db.db_path = abs.string();
    db.manifest_path      = (abs / "MANIFEST").string();
    db.wal_path           = (abs / "WAL.current").string();
    db.catalog_dir        = (abs / "catalog").string();
    db.catalog_meta_path  = (abs / "catalog" / "meta.ndjson").string();
    db.blobs_dir          = (abs / "blobs").string();
    db.thumbs_dir         = (abs / "thumbs").string();

    return db;
}

bool ImageDB::Init(){
    std::string db_path = this->db_path;

    if(!std::filesystem::exists(db_path)) {
        stderr::cout<<"Database doesn't exists at the specified path \n";
        return true;
    }

    // create the blob_dir
    std::string blob_dir = db_path + "/blobs";
    ensure_dirs(blob_dir);

    // create thumbnails dir
    std::string thumbs_dir = db_path + "/thumbs";
    ensure_dirs(thumbs_dir);

    // create catalog dir
    std::string catalog_dir = db_path + "/catalog";

    return true;


}

bool ImageDB::ImportFile(const std::string& file){
    std::string hash = sha256_file(file);
    std::string blob_dir = db_path + "/blobs" + hash.substr(0,2);
    std::string blob_path = blob_dir + "/" + hash;

    if(std::filesystem::exists(blob_path)) {
        std::cout<<"Already present (sha256 match). Skipped.\n";
    }

    ensure_dirs(blob_dir);
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

    append_json_line(catalog_path + "/meta.ndjson", meta_to_json(m));
    make_thumbnail_256(file, thumbs_path + "/" + m.image_id + "_256.jpg");

    std::cout << "Imported: " << m.image_id << " sha256=" << hash << "\n";
    return true;
};