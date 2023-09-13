#pragma

namespace config
{
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
    namespace Syscall
    {
        bool active;
        bool save;
    }
    namespace dbg
    {
        typedef struct _TDR_SET
        {
            bool active;
            int statue;
            int type;
            int size;
            char addr[50];
        }DrSet;

        int curtChoose = 0;
        DrSet Dr[4];
    }
}
