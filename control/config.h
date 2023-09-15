#pragma

namespace config
{
    namespace global
    {
        plugin plugin_;
        ProcessItem targetProcess;
        ProcessItem injectProcess;
        uint64_t lpPluginDispatch;
    }
    namespace process
    {
        namespace thread
        {
            bool bShow;
            uint32_t pid;
        }
        namespace veh
        {
            bool bShow;
            uint32_t pid;
        }
    }
    namespace syscall
    {
        std::map<std::string, std::map<std::string, uint64_t>> monitor{};
        bool active;
        bool save;
    }
    namespace dbg
    {
        typedef struct _TDR_SET
        {
            bool active;        //是否启用
            int statue;         //是否添加/移除
            int type;           //断点类型:可执行、读写、写
            int size;           //断点大小
            char addr[50];      //断点地址
        }DrSet;

        int curtChoose = 0;
        DrSet Dr[4];
    }
}
