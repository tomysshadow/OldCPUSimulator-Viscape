#include "shared.h"
#include <string>
#include <windows.h>

static LPCSTR OLD_CPU_SIMULATOR_COMMAND_LINE_PATH = "OldCPUSimulator_commandLine.txt";

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, PSTR commandLine, INT commandShow) {
	int result = 0;

	HANDLE applicationMutex = CreateMutex(NULL, FALSE, "Old CPU Simulator");

	if (applicationMutex && applicationMutex != INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			// times out after 10 seconds to prevent hang in event of failure
			DWORD wait = WaitForSingleObject(applicationMutex, 10000);

			if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED) {
				showLastError("Failed to Wait For Single Object");
				result = -2;
			}

			if (!ReleaseMutex(applicationMutex)) {
				showLastError("Failed to Release Mutex");
				result = -2;
			}
		}

		if (!CloseHandle(applicationMutex)) {
			showLastError("Failed to Close Handle");
			result = -2;
			goto error5;
		}

		applicationMutex = NULL;

		error5:
		if (result) {
			return result;
		}
	}

	HANDLE oldCPUSimulatorCommandLineFile = CreateFile(OLD_CPU_SIMULATOR_COMMAND_LINE_PATH, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (!oldCPUSimulatorCommandLineFile || oldCPUSimulatorCommandLineFile == INVALID_HANDLE_VALUE) {
		showLastError("Failed to Create File");
		result = -1;
		goto error;
	}

	const DWORD OLD_CPU_SIMULATOR_COMMAND_LINE_SIZE = 8192;
	CHAR oldCPUSimulatorCommandLine[OLD_CPU_SIMULATOR_COMMAND_LINE_SIZE] = "";

	DWORD numberOfBytesRead = 0;

	if (!ReadFile(oldCPUSimulatorCommandLineFile, oldCPUSimulatorCommandLine, OLD_CPU_SIMULATOR_COMMAND_LINE_SIZE - 1, &numberOfBytesRead, NULL)) {
		showLastError("Failed to Read File");
		result = -1;
	}

	if (!CloseHandle(oldCPUSimulatorCommandLineFile)) {
		showLastError("Failed to Close Handle");
		result = -1;
		goto error4;
	}

	oldCPUSimulatorCommandLineFile = NULL;

	error4:
	if (result) {
		return result;
	}

	{
		result = -1;

		std::string oldCPUSimulatorViscapeCommandLine = std::string(oldCPUSimulatorCommandLine) + " " + std::string(commandLine);

		SIZE_T _oldCPUSimulatorViscapeCommandLineSize = oldCPUSimulatorViscapeCommandLine.size() + 1;
		LPSTR _oldCPUSimulatorViscapeCommandLine = new CHAR[_oldCPUSimulatorViscapeCommandLineSize];

		if (!_oldCPUSimulatorViscapeCommandLine) {
			showLastError("Failed to Allocate oldCPUSimulatorViscapeCommandLine");
			goto error;
		}

		if (strncpy_s(_oldCPUSimulatorViscapeCommandLine, _oldCPUSimulatorViscapeCommandLineSize, oldCPUSimulatorViscapeCommandLine.c_str(), _oldCPUSimulatorViscapeCommandLineSize)) {
			showLastError("Failed to Copy String");
			goto error2;
		}

		{
			STARTUPINFO startupInfo = {};
			startupInfo.cb = sizeof(startupInfo);

			PROCESS_INFORMATION processInformation = {};

			// create the processHandle, fail if we can't
			if (!CreateProcess(NULL, _oldCPUSimulatorViscapeCommandLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInformation)) {
				showLastError("Failed to Create Process");
				goto error2;
			}

			if (processInformation.hProcess) {
				if (!CloseHandle(processInformation.hProcess)) {
					showLastError("Failed to Close Handle");
					goto error3;
				}

				processInformation.hProcess = NULL;
			}

			error3:
			if (processInformation.hThread) {
				if (!CloseHandle(processInformation.hThread)) {
					showLastError("Failed to Close Handle");
					goto error2;
				}

				processInformation.hThread = NULL;
			}
		}

		result = 0;
		error2:
		delete[] _oldCPUSimulatorViscapeCommandLine;
		_oldCPUSimulatorViscapeCommandLine = NULL;
		_oldCPUSimulatorViscapeCommandLineSize = 0;
	}
	error:
	return result;
}