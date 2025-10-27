#include<string>
#include<algorithm>
#include <db.h>

struct ParsedArgs {
    std::string cmd;
    std::string db_path;
};

char* getCmdOption(char** begin, char** end, const std::string& option){
    char** itr = std::find(begin, end, option);

    if(itr != end && ++itr !=end) return *itr;

    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

ParsedArgs parse_args(int argc, char** argv){

}

int main(int argc, char **argv){
    ParsedArgs args = parse_args(argc, argv);

    if(args.cmd == "init") {
        return ImageDB::Open(args.db_path).Init() ? 0 : 1;
    }
}