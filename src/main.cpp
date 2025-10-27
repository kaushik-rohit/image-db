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
    ParsedArgs args;
    
    if(cmdOptionExists(argv, argv+argc, "-cmd")) {
        args.cmd = getCmdOption(argv, argv + argc, "-cmd");
    } else {
        throw std::runtime_error("Usage: -cmd is needed");
    }

    if(cmdOptionExists(argv, argv+argc, "-root")){
        args.db_path = getCmdOption(argv, argv+argc, "-root");
    } else {
        throw std::runtime_error("Usage: -root is needed");
    }

    return args;
}

int main(int argc, char **argv){
    ParsedArgs args = parse_args(argc, argv);

    if(args.cmd == "init") {
        return ImageDB::Open(args.db_path).Init() ? 0 : 1;
    }
}