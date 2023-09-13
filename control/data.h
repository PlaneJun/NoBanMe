#pragma once
namespace data
{
    namespace global
    {
        pipe pipe_;
        bool inject;
        uint32_t target;
        uint64_t fnDispatch;
        char plugin_path[256];
    }

    namespace Syscall
    {
        std::map<std::string, std::map<std::string, uint64_t>> monitor{};
    }
}
