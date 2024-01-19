#include <string>
#include <filesystem>
#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>
#include "luaApi.h"

void format_table_to_string(lua_State* L, int idx, std::string& res)
{
	if (!lua_istable(L, idx))
	{
		return;
	}
	else
	{
		res.append("{");

		// 将table拷贝到栈顶
		lua_pushvalue(L, idx);
		int it = lua_gettop(L);

		// 压入一个nil值，充当起始的key
		lua_pushnil(L);
		while (lua_next(L, it))
		{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
			lua_pushvalue(L, -2);

			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table
			if (!lua_istable(L, -2))
			{
				// 不是table就直接取出来就行
				const char* key = lua_tostring(L, -1);
				char t[256] = {};
				if (lua_isboolean(L, -2))
				{
					sprintf_s(t, "%s=%s,", key, lua_toboolean(L, -2) ? "true" : "false");
				}
				else if (lua_isfunction(L, -2))
				{
					sprintf_s(t, "%s=%s,", key, "function");
				}
				else
				{
					sprintf_s(t, "%s=%s,", key, lua_tostring(L, -2));
				}
				res.append(t);

			}
			else
			{
				const char* key = lua_tostring(L, -1);
				res.append(key);
				res.append("=");

				// 此刻-2 => value
				format_table_to_string(L, -2, res);

				//lua_pop(L, 1);
				res += ',';
			}

			// 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
			lua_pop(L, 2);
			// 现在的栈：-1 => key; index => table
		}

		// 现在的栈：index => table （最后 lua_next 返回 0 的时候它已经把上一次留下的 key 给弹出了）
		res.substr(0, res.length() - 1);
		res.append("}");

		// 弹出上面被拷贝的table
		lua_pop(L, 1);
	}
}


int lua_api::l_print(lua_State* L)
{
	int n = lua_gettop(L);
	std::string strPrint;
	for (int p = 1; p <= n; p++)
	{
		if (lua_istable(L, p))
		{
			format_table_to_string(L, p, strPrint);
		}
		else if (lua_isboolean(L, p))
		{
			strPrint.append(lua_toboolean(L, p) ? "true" : "false");
		}
		else if (lua_isfunction(L, p))
		{
			strPrint.append("function");
		}
		else if (lua_isnumber(L, p))
		{
			uint64_t ud = lua_tonumber(L, p);
			strPrint.append(std::to_string(ud));
		}
		else if (lua_isinteger(L, p))
		{
			uint64_t ud = lua_tointeger(L, p);
			strPrint.append(std::to_string(ud));
		}
		else if (lua_isstring(L, p))
		{
			strPrint.append(lua_tostring(L, p));
		}
		else
		{
			uint64_t ud = (uint64_t)lua_touserdata(L, p);
			strPrint.append(std::to_string(ud));
		}
		strPrint.append("\t");
	}

	strPrint.substr(0, strPrint.length() - 1);
	printf("%s\n", strPrint.c_str());
	return 0;
}

int lua_api::l_msgbox(lua_State* L)
{
	HWND hWnd = (HWND)lua_touserdata(L, 1);
	std::string text = std::string(lua_tostring(L, 2));
	std::string caption = std::string(lua_tostring(L, 3));
	UINT type = lua_tointeger(L, 4);
	MessageBoxA(hWnd, text.c_str(), caption.c_str(), type);
	return 0;
}

int lua_api::l_getmodulebase(lua_State* L)
{
	uint32_t pid = lua_tointeger(L, 1);
	std::string module_name = std::string(lua_tostring(L, 2));

	uint64_t base = 0;
	HANDLE hSnapshot_proc = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	if (hSnapshot_proc)
	{
		MODULEENTRY32  me32 = { sizeof(MODULEENTRY32) };
		bool bprocess = Module32First(hSnapshot_proc, &me32);
		while (bprocess)
		{
			auto mn = std::filesystem::path(me32.szModule).filename().string();
			if (mn._Equal(module_name))
			{
				base = (uint64_t)me32.hModule;
				break;
			}
			bprocess = Module32Next(hSnapshot_proc, &me32);
		}
		CloseHandle(hSnapshot_proc);
	}

	lua_pushlightuserdata(L, (PVOID)base);
	return 1;
}

int lua_api::l_readmem(lua_State* L)
{
	return 0;
}

int lua_api::l_writemem(lua_State* L)
{

	return 0;
}

int lua_api::l_sleep(lua_State* L)
{
	uint32_t time = lua_tointeger(L, 1);
	Sleep(time);
	return 0;
}
