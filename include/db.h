#include<string>

class ImageDB {
public:
    static ImageDB Open(const std::string& db_path);
    bool Init();
    bool ImportFile(const std::string& file);

};