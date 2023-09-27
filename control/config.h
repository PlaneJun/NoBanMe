#pragma

namespace config
{
    namespace global
    {
        plugin plugin_;
        uint64_t lpPluginDispatch;
    }
    namespace process
    {
        namespace memory
        {
            bool bShow;
            uint32_t pid;
        }
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
