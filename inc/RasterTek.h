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
	uchar mod = 1;
};

extern RTUserArgs RTArgs;

#define CHECK_RT_TEST_NUM(test_num) ( (test_num == RTArgs.test) ? true: false )
#define CHECK_RT_TEST_IN_RANGE(test_num) ( ((test_num >= RTArgs.test) && (test_num <= RTArgs.end)) ? true: false )
#define CHECK_RT_API(test_api) ( (test_api == RTArgs.api) ? true: false )

#define WCHAR2CHAR(var_wchar_ptr, var_char_ptr) {\
	int size = WideCharToMultiByte(CP_UTF8, 0, var_wchar_ptr, -1, NULL, 0, NULL, NULL);\
	var_char_ptr = new char[size];\
	WideCharToMultiByte(CP_UTF8, 0, var_wchar_ptr, -1, var_char_ptr, size, NULL, NULL);\
}
