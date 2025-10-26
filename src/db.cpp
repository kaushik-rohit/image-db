#include <db.h>
#include <string>
#include <meta.h>
#include <sha.h>

bool ImageDB::Init(){

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