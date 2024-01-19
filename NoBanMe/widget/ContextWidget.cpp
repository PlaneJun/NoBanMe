#include "ContextWidget.h"
#include <imgui.h>
#include "../render/render.h"
#include "../../common/utils/utils.h"

void ContextWidget::OnPaint()
{
    ImGui::SeparatorText(u8"寄存器"); ImGui::SameLine();
    render::get_instasnce()->HelpMarker(u8"当前显示的上下文为断点执行后数据\n");
    std::vector<std::string> headers = { u8"Reg" ,u8"Value" };
    std::vector<std::pair<std::string, std::string>> text = {
        {"RAX",utils::conver::IntegerTohex(DataSource_.Rax)},
        {"RBX",utils::conver::IntegerTohex(DataSource_.Rbx)},
        {"RCX",utils::conver::IntegerTohex(DataSource_.Rcx) },
        {"RDX",utils::conver::IntegerTohex(DataSource_.Rdx)},
        {"RBP",utils::conver::IntegerTohex(DataSource_.Rbp)},
        {"RSP",utils::conver::IntegerTohex(DataSource_.Rsp)},
        {"RSI",utils::conver::IntegerTohex(DataSource_.Rsi)},
        {"RDI",utils::conver::IntegerTohex(DataSource_.Rdi)},
        { "R8" ,utils::conver::IntegerTohex(DataSource_.R8) },
        { "R9",utils::conver::IntegerTohex(DataSource_.R9) },
        { "R10",utils::conver::IntegerTohex(DataSource_.R10) },
        { "R11",utils::conver::IntegerTohex(DataSource_.R11) },
        { "R12",utils::conver::IntegerTohex(DataSource_.R12) },
        { "R13" ,utils::conver::IntegerTohex(DataSource_.R13)},
        { "R14",utils::conver::IntegerTohex(DataSource_.R14) },
        { "R15",utils::conver::IntegerTohex(DataSource_.R15) },
        { "RIP" ,utils::conver::IntegerTohex(DataSource_.Rip) },
        { "RFLAGS" ,utils::conver::IntegerTohex(DataSource_.EFlags) },
        {"Xmm0",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm0)},
        {"Xmm1",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm1)},
        {"Xmm2",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm2)},
        {"Xmm3",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm3)},
        {"Xmm4",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm4)},
        {"Xmm5",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm5)},
        {"Xmm6",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm6)},
        {"Xmm7",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm7)},
        {"Xmm8",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm8)},
        {"Xmm9",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm9)},
        {"Xmm10",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm10)},
        {"Xmm11",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm11)},
        {"Xmm12",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm12)},
        {"Xmm13",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm13)},
        {"Xmm14",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm14)},
        {"Xmm15",utils::conver::IntegerTohex(*(uint64_t*)&DataSource_.Xmm15)}
    };
    render::get_instasnce()->AddListBox("##context", selected_, headers, text);
}

void ContextWidget::SetDataSource(CONTEXT data)
{
    DataSource_ = data;
}