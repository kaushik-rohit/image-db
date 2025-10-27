#include<string>

class ImageDB {
public:
    static ImageDB Open(const std::string& db_path);
    bool Init();
    bool ImportFile(const std::string& file);

    std::string db_root;
    std::string manifest_path;
    std::string wal_path;
    std::string catalog_dir;
    std::string catalog_meta_path;
    std::string blobs_dir;
    std::string thumbs_dir;
    bool is_initialized;
};
