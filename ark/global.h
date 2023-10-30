#pragma once
#include <stdint.h>
#include "plugin/plugin.h"

namespace global
{
    namespace plugin
    {
        inline  ::plugin plugin_{};
        inline  uint64_t lpPluginDispatch{};
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

        inline int curtChoose{};
        inline DrSet Dr[4]{};
    }
}
