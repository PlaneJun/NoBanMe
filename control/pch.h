#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <Zydis.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <map>
#include <sstream>
#include <filesystem>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "defs.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_theme.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#include <tchar.h>
#include <imgui_memory_editor.h>

#include "pipe.h"
#include "data.h"
#include "config.h"

#include "render.h"
#include "utils.h"







