#include<string>
#include "db.h"

struct ParsedArgs {
    std::string cmd;
    std::string db_path;
};

ParsedArgs parse_args(int argc, char** argv){

}

int main(int argc, char **argv){
    ParsedArgs args = parse_args(argc, argv);

    if(args.cmd == "init") {
        return ImageDB::Open(args.db_path).Init() ? 0 : 1;
    }
}