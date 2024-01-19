#pragma once

extern "C"
{
	#include "../../3part/lua/include/lua.h"
}

namespace lua_api
{
	int l_print(lua_State* L);

	int l_msgbox(lua_State* L);

	int l_getmodulebase(lua_State* L);

	int l_readmem(lua_State* L);

	int l_writemem(lua_State* L);

	int l_sleep(lua_State* L);
}


