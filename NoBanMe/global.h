#pragma once
#include <stdint.h>
#include "../common/ipc/ipc.h"

namespace global
{
    namespace plugin
    {
        inline  ::IPCCom plugin_{};
        inline  uint64_t lpPluginDispatch{};
    }

    namespace dbg
    {
        typedef struct _TDR_SET
        {
            bool active;        //�Ƿ�����
            int statue;         //�Ƿ����/�Ƴ�
            int type;           //�ϵ�����:��ִ�С���д��д
            int size;           //�ϵ��С
            char addr[50];      //�ϵ��ַ
        }DrSet;

        inline int curtChoose{};
        inline DrSet Dr[4]{};
    }
}
