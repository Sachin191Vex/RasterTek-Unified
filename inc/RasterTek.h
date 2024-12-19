// RasterTek.h : Include file for standard system include files,
// or project specific include files.
#pragma once

// Excludes less frequently used APIs such as Cryptography, DDE, RPC, Shell, and Windows Sockets.
// This reduces the size of the Win32 header files
// Ref: https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <iostream>

#define		RT_OK		1
#define		RT_ERROR	0
