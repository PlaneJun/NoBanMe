#pragma once
#include <string>

extern "C"
{
#include "../3rdParty/lua/include/lua.h"
#include "../3rdParty/lua/include/lauxlib.h"
#include "../3rdParty/lua/include/lualib.h"
}

#pragma comment(lib,"./3rdParty/lua/lua54.lib")

class LuaScript
{
public:
	LuaScript();

	~LuaScript();

	std::string get_last_error_msg() const { return last_error_; }

	bool exec_from_string(std::string script);

	bool exec_from_file(std::string filepath);

private:
	std::string last_error_;

private:
	lua_State* lua_state_;

};
