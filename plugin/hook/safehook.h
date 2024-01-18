#pragma once
#include <vector>
#include <Windows.h>
#include <stdint.h>

class safehook
{
public:
	typedef struct _THOOK_CONTEXT
	{
		M128A xmm3;
		M128A xmm2;
		M128A xmm1;
		M128A xmm0;
		uint64_t eflags;
		uint64_t r15;
		uint64_t r14;
		uint64_t r13;
		uint64_t r12;
		uint64_t r11;
		uint64_t r10;
		uint64_t r9;
		uint64_t r8;
		uint64_t rdi;
		uint64_t rsi;
		uint64_t rbp;
		uint64_t rdx;
		uint64_t rcx;
		uint64_t rbx;
		uint64_t rax;
		uint64_t rsp;
	}HookContext;

	typedef struct _THOOK_INFO
	{
		uint32_t id;
		uint64_t hook_addr;			// hook�ĵ�ַ
		uint16_t patch_len;			// �޸�hook��ַ���ֽ���
		uint8_t patch_opcode[20];	// ����hook��ַԭ�ֽ�
		uint64_t disp_addr;			// �ַ������ĵ�ַ
		uint64_t proxy_addr;			// ��ʵִ�еĵ�ַ
		bool	running;	// ����ִ��
	}HookInfo;

public:
	// ���hook��
	void add_hook(uint64_t hook_addr, uint64_t proxy_func);

	// ȡhook��Ϣ
	safehook::HookInfo get_hook_info(uint64_t hook_addr);

	// ����hook�ɹ�����
	uint32_t hook();

	//ժ��
	bool unhook(uint64_t hook_addr);

private:

	uint32_t dispatch_func_length_;
	std::vector<HookInfo> hooklist_;

private:
	// ȡ��Ҫpatch�Ĵ�С
	uint32_t get_patch_len(uint64_t addr);

	//ָ��λ������
	uint64_t alloc_mem_nearby(uint64_t addr, uint32_t Size);
};
