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

		// ��table������ջ��
		lua_pushvalue(L, idx);
		int it = lua_gettop(L);

		// ѹ��һ��nilֵ���䵱��ʼ��key
		lua_pushnil(L);
		while (lua_next(L, it))
		{
			// ���ڵ�ջ��-1 => value; -2 => key; index => table
			// ����һ�� key ��ջ����Ȼ������� lua_tostring �Ͳ���ı�ԭʼ�� key ֵ��
			lua_pushvalue(L, -2);

			// ���ڵ�ջ��-1 => key; -2 => value; -3 => key; index => table
			if (!lua_istable(L, -2))
			{
				// ����table��ֱ��ȡ��������
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

				// �˿�-2 => value
				format_table_to_string(L, -2, res);

				//lua_pop(L, 1);
				res += ',';
			}

			// ���� value �Ϳ����� key������ԭʼ�� key ��Ϊ��һ�� lua_next �Ĳ���
			lua_pop(L, 2);
			// ���ڵ�ջ��-1 => key; index => table
		}

		// ���ڵ�ջ��index => table ����� lua_next ���� 0 ��ʱ�����Ѿ�����һ�����µ� key �������ˣ�
		res.substr(0, res.length() - 1);
		res.append("}");

		// �������汻������table
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
