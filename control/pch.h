#pragma once
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup")
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define NOMINMAX
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <stdint.h>


#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <filesystem>
#include <functional>

#include <Zydis.h>
#include "zydis_wrapper.h"
#include <gdiplus.h>
#include <DbgHelp.h>
#include <Shlobj.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_theme.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#include <tchar.h>
#include <imgui_memory_editor.h>


#include "defs.h"
#include "struct.h"
#include "memstub.h"
#include "md5.h"
#include "EasyPdb.h"
#include "plugin.h"

#include "render.h"
#include "utils.h"
//Process
#include "process.h"
#include "module.h"
#include "thread.h"
#include "memory.h"
#include "vehhandler.h"
#include "window.h"
//Debugger
#include "debugger.h"
#include "config.h"

#include "DisassemblyWidget.h"
#include "ProcessWidget.h"









