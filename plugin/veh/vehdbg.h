#pragma once
#include <map>
#include <Windows.h>
#include <stdint.h>

class vehdbg
{
public:
	using cbDbgHandler = void(*)(uint8_t, PCONTEXT&);

	enum DBG_TYPE
	{
		TYPE_EXECUTE,
		TYPE_READWRITE,
		TYPE_WRITE
	};
	enum DBG_SIZE
	{
		SIZE_1,
		SIZE_2,
		SIZE_4,
		SIZE_8
	};

	struct DBG_PARAMS
	{
		uint8_t index;
		DBG_TYPE type;
		DBG_SIZE size;
		uint64_t address;
	};

public:
	void init();

	void close();

	void set_callback(cbDbgHandler cb);

	int set_break(uint8_t dr_index, uint64_t addr, DBG_SIZE len, DBG_TYPE type);

	bool unset_hardbreak(uint8_t dr_index);

	bool get_hardBreak_by_index(uint8_t index, std::pair<struct DBG_PARAMS, bool>& out);

	void set_hardBreak_state(uint8_t index, bool state);

	static void Dispatch(uint8_t id, PCONTEXT& ctx);
private:
	static cbDbgHandler callback_;
	PVOID hHandler_;
	std::map<uint8_t, std::pair<struct DBG_PARAMS, bool>> hbk_list_;
	

	void set_bits(uint64_t& dw, int lowBit, int bits, int newValue);
};