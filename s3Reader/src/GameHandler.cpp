#include "GameHandler.h"

GameHandler::GameHandler(std::string gameName, std::string windowName) : m_gameName{gameName}, m_windowName{windowName} {};

bool GameHandler::find()
{
    const char* windowName = &m_windowName[0];
	HWND gameWindow = FindWindowA(NULL, windowName);
	if (gameWindow == NULL) {
		std::cout << "no game found, please start the game\n";
		std::cin.get();
		return 1;
	}
	else {
		std::cout << "game found\n";
	}

	DWORD processId = NULL;  // ID of our Game
	GetWindowThreadProcessId(gameWindow, &processId);
	m_processHandle = NULL;
	m_processHandle = OpenProcess(PROCESS_VM_READ, FALSE, processId);
	if (m_processHandle == INVALID_HANDLE_VALUE || m_processHandle == NULL) {
		std::cout << "failed to open process " << processId << " (processId) with processHandle " << m_processHandle << "\n";
		std::cin.get();
		return 1;
	}
	else {
		std::cout << "processHandle found\n";
	}

	char* gameName = &m_gameName[0];
	m_gameBaseAddress = getModuleBaseAddress(processId, _T(gameName));
	std::cout << "gameBaseAddress = " << std::hex << m_gameBaseAddress << std::dec << "\n";

	return 0;
};

int GameHandler::readInt(unsigned int offset)
{
	uintptr_t value = NULL;
	ReadProcessMemory(m_processHandle, (BYTE*)(m_gameBaseAddress + offset), &value, sizeof(value), NULL);

	return value;
};

int GameHandler::readInt(std::vector<unsigned int> offset)
{
	uintptr_t value = m_gameBaseAddress;
	for (size_t i = 0; i < offset.size(); i++)
		ReadProcessMemory(m_processHandle, (BYTE*)((int)value +  offset[i]), &value, sizeof(value), NULL);

	return value;
};

std::string GameHandler::readString(std::vector<unsigned int> offset)
{
	uintptr_t value = m_gameBaseAddress;
	for (size_t i = 0; i < offset.size() - 1; i++)
		ReadProcessMemory(m_processHandle, (BYTE*)((int)value +  offset[i]), &value, sizeof(value), NULL);

	char buffer[32];
	bool bReturn = ReadProcessMemory(m_processHandle, (BYTE*)((int)value + offset[offset.size() - 1]), buffer, 31, NULL);
	if (bReturn == 0)  // error condition: no player name on the heap
		buffer[0] = { '\0' };
	
	return std::string(buffer);
};

uintptr_t GameHandler::getModuleBaseAddress(DWORD processId, TCHAR* moduleName)
{
	uintptr_t moduleBaseAddress = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);  // snapshot of all modules within process
	MODULEENTRY32 moduleEntry = { 0 };
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(snapshot, &moduleEntry)) {  // store first module in moduleEntry
		do {
			if (_tcscmp(moduleEntry.szModule, moduleName) == 0) {  // search for the module we are looking for
				moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
				break;
			}
		} while (Module32Next(snapshot, &moduleEntry));  // go through module entries in snapshot and store in moduleEntry
	}
	CloseHandle(snapshot);

	return moduleBaseAddress;
};