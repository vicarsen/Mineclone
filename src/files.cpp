#include "files.h"

#include <fstream>

DEFINE_LOG_CATEGORY(Files, spdlog::level::trace, LOGFILE("Files.txt"));

namespace Files
{
    std::string ReadFile(const std::string& path)
    {
        TRACE(Files, "[ReadFile] (#path:{})", path);

        std::ifstream file(path);
        if(!file)
        {
            ERROR(Files, "[ReadFile] (path:{}) file doesn't exist", path);
            return "";
        }

        std::stringstream ss;
        ss << file.rdbuf();

        TRACE(Files, "[ReadFile] return");
        return ss.str();
    }
};
 
