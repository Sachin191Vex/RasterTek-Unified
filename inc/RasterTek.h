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

// structure to specify how to run tests - parsed by command line arguments
typedef enum { API_DX11 = 1, API_DX12 = 2, API_VK = 3, API_OGL = 3 } RTApi;
typedef unsigned char uchar;

struct RTUserArgs {
	RTApi api = API_DX11;
	uchar test = 5;
	uchar end = 5;
};

extern RTUserArgs RTArgs;