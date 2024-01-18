#pragma once
#include <Windows.h>

class ContextWidget
{
public:
    void OnPaint();

    void SetDataSource(CONTEXT data);
private:
    int selected_ = -1;
    CONTEXT DataSource_;

};