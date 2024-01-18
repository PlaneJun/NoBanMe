#include <iostream>
#include <filesystem>
#include <Windows.h>
#include "luaApi.h"
#include "luaScript.h"



const struct luaL_Reg printlib[] = {
	{"print", lua_api::l_print},
	{"msgbox",lua_api::l_msgbox},
	{"getmodulebase",lua_api::l_getmodulebase},
	{"reamem",lua_api::l_readmem},
	{"writemem",lua_api::l_writemem},
	{"sleep",lua_api::l_sleep},
	{NULL, NULL} /* end of array */
};

LuaScript::LuaScript()
{
	if (!lua_state_)
	{
		lua_state_ = luaL_newstate();
		luaL_openlibs(lua_state_);
		// ÷ÿ∂®“Âprint
		lua_getglobal(lua_state_, "_G");
		luaL_setfuncs(lua_state_, printlib, 0);
		lua_pop(lua_state_, 1);
	}
}

LuaScript::~LuaScript()
{
	if (!lua_state_)
	{
		lua_close(lua_state_);
	}
}

bool LuaScript::exec_from_string(std::string script)
{
	int status = luaL_dostring(lua_state_,script.c_str());
	if (status != LUA_OK)
	{
		last_error_ = lua_tostring(lua_state_,-1);
	}

	return status == LUA_OK;
}

bool LuaScript::exec_from_file(std::string filepath)
{
	if (!std::filesystem::exists(filepath))
	{
		return false;
	}

	int status = luaL_dofile(lua_state_, filepath.c_str());
	if (status != LUA_OK)
	{
		last_error_ = lua_tostring(lua_state_, -1);
	}

	return status == LUA_OK;
}