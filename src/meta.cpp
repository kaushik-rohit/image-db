#include <meta.h>
#include "json.hpp"

using json = nlohmann::json;

std::string meta_to_json(const ImageMeta& m) {
    json obj;
    obj["image_id"] = m.image_id;
    obj["mime"] = m.mime;
    obj["sha256"] = m.sha256;
    obj["width"] = m.width;
    obj["height"] = m.height;
    obj["created_at"] = m.created_unix;

    return obj.dump(4);

}