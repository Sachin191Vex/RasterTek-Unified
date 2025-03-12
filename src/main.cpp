////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp : Defines the entry point for the application.
////////////////////////////////////////////////////////////////////////////////
#include "RasterTek.h"
#include "systemclass.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <io.h>
#include <stdexcept> // For std::invalid_argument and std::out_of_range
#include <codecvt>

RTUserArgs RTArgs;

// --------------------------------------------------------------------------------------------------------------------
// Function to display the help message
void RedirectIOToConsole() {
	// Redirect standard output streams to the console
	FILE* fp;
	AllocConsole();
	_wfreopen_s(&fp, L"CONOUT$", L"w", stdout);
	_wfreopen_s(&fp, L"CONOUT$", L"w", stderr);
	_wfreopen_s(&fp, L"CONIN$", L"r", stdin);

	// Enable wide-character output on std::wcout
	std::ios::sync_with_stdio();
}

void displayHelp() {
	std::wcout << L"Run from build directory to access to resources (..\\shaders, ..\\data\\models, ..\\data\\textures)\n";
	std::wcout << L"Usage: [options]\n";
	std::wcout << L"  -h or --help   Display this help message\n";
	std::wcout << L"  --test <>      Test number to run (default=5)\n";
	std::wcout << L"  --api <>       Specify api: API_DX11=1 (default), API_DX12=2, API_VK=3, API_OGL=3\n";
	std::wcout << L"  --end <>       End test number to end (inclusive) (default=5)\n";
	std::wcout << L"  --dir <>       Path to resources (default .) - not yet supported\n";
}

// Function to parse arguments
#define CHECK_AND_ASSIGN(argname, assign_type, assign_to) {\
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;\
	std::wstring wstr = converter.from_bytes(argname);\
	auto argIt = std::find(args.begin(), args.end(), wstr);\
	if (argIt != args.end()) {\
		validArgumentFound = true;\
		if ((argIt + 1) != args.end()) {\
			auto argValue = *(argIt + 1);\
			try {\
				assign_to = static_cast<assign_type>(std::stoi(argValue));\
			}\
			catch (const std::invalid_argument& e) {\
				std::cout << "Error: Invalid value for " ## argname ## ". Must be a number.\n";\
				return(RT_ERROR);\
			}\
			catch (const std::out_of_range& e) {\
				std::cout << "Error: Value for " ## argname ## " is out of range.\n";\
				return(RT_ERROR);\
			}\
		}\
		else {\
			std::cout << "Error: " ## argname ## " requires a value\n";\
			return(RT_ERROR);\
		}\
	}\
}

int parseArguments(LPWSTR lpCmdLine) {
	// Convert LPWSTR to a std::wstring
	std::wstring cmdLine(lpCmdLine);

	// Split the command line into arguments
	std::vector<std::wstring> args;
	size_t pos = 0;
	while ((pos = cmdLine.find(L' ')) != std::wstring::npos) {
		args.push_back(cmdLine.substr(0, pos));
		cmdLine.erase(0, pos + 1);
	}
	if (!cmdLine.empty()) {
		args.push_back(cmdLine);
	}

	// Flags to track valid arguments
	bool validArgumentFound = false;

	// Check for -h or --help
	if (std::find(args.begin(), args.end(), L"-h") != args.end() ||
		std::find(args.begin(), args.end(), L"--help") != args.end()) {
		displayHelp();
		validArgumentFound = true;
		return(RT_ERROR);
	}

	// Check for other parameters
 	CHECK_AND_ASSIGN("--api", RTApi, RTArgs.api);
	CHECK_AND_ASSIGN("--test", uchar, RTArgs.test);
	CHECK_AND_ASSIGN("--end", uchar, RTArgs.end);

	if ( (!args.empty()) && (validArgumentFound != true) ) {
		std::wcout << L"No valid arguments provided. Use -h or --help for help.\n";
		return(RT_ERROR);
	}

	return(RT_OK);
}

// --------------------------------------------------------------------------------------------------------------------
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	SystemClass* system = nullptr;
	int result;

	// Create a new console
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
	    // MessageBox(NULL, "Failed to attach to a console!", "Error", MB_OK);
	}
	RedirectIOToConsole();

	result = parseArguments(lpCmdLine);
	if (result == RT_ERROR) { exit(0); }

	system = new SystemClass;
	result = system->Initialize();
	if (result == RT_OK) {
		system->Run();
	}

	system->Shutdown();
	delete system;
	system = nullptr;
}

// --------------------------------------------------------------------------------------------------------------------
