#pragma once

//
// Outputs a string to the plugin log file, debug output and any attached console.
//
void Trace(const char* format, ...);

//
// Outputs an annotated string to the log output.
//
#define FuncTrace(x, ...) Trace(__FUNCTION__ ## x, __VA_ARGS__)