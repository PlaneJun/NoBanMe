#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <Zydis.h>
#include <gdiplus.h>
#include <DbgHelp.h>
#include <atlconv.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_theme.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#include <tchar.h>
#include <imgui_memory_editor.h>

#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <filesystem>
#include "defs.h"
#include "struct.h"
#include "md5.h"
#include "EasyPdb.h"
#include "pipe.h"
#include "data.h"
#include "config.h"
#include "render.h"


//Process
#include "process.h"
#include "module.h"
#include "thread.h"
#include "vehhandler.h"

#include "utils.h"
#include "pattern.h"
//Debugger
#include "debugger.h"









