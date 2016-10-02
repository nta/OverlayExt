#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <stdint.h>

#include <AyriaLog.h>

// helpers
#include <string>

std::wstring StringToWide(const std::string& narrow);
std::string StringToNarrow(const std::wstring& wide);