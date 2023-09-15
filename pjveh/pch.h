#pragma once

#include <Windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <winnt.h>
#include <DbgHelp.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <stdarg.h>
#include <time.h>
#pragma comment (lib, "imagehlp.lib")
#pragma comment( lib, "ntdll.lib" )

#include "InstrumentationCallback.hpp"
#include "log.h"
#include "veh.h"
#include "defs.h"
#include "plugin.h"
