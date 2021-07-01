#include "GameHandler.hpp"

GameHandler::GameHandler(std::string gameName, std::string windowName)
    : gameName{ gameName }, windowName{ windowName } {};

bool GameHandler::find()
{
    const char* buffer = &windowName[0];
    HWND gameWindow = FindWindowA(NULL, buffer);
    if (gameWindow == NULL) {
        std::cout << "no game found, please start the game\n";
        std::cin.get();
        return 1;
    } else {
        std::cout << "game found\n";
    }

    DWORD processId = NULL; // ID of our Game
    GetWindowThreadProcessId(gameWindow, &processId);
    processHandle = NULL;
    processHandle = OpenProcess(PROCESS_VM_READ, FALSE, processId);
    if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) {
        std::cout << "failed to open process " << processId << " (processId) with processHandle " << processHandle
                  << "\n";
        std::cin.get();
        return 1;
    } else {
        std::cout << "processHandle found\n";
    }

    char* buffer2 = &gameName[0];
    gameBaseAddress = getModuleBaseAddress(processId, _T(buffer2));
    std::cout << "gameBaseAddress = " << std::hex << gameBaseAddress << std::dec << "\n";

    return 0;
};

int GameHandler::readInt(int offset)
{
    uintptr_t value = NULL;
    ReadProcessMemory(processHandle, (BYTE*)(gameBaseAddress + offset), &value, sizeof(value), NULL);

    return value;
};

int GameHandler::readInt(std::vector<int> offset)
{
    uintptr_t value = gameBaseAddress;
    for (size_t i = 0; i < offset.size(); i++)
        ReadProcessMemory(processHandle, (BYTE*)((int)value + offset[i]), &value, sizeof(value), NULL);

    return value;
};

std::string GameHandler::readString(std::vector<int> offset)
{
    uintptr_t value = gameBaseAddress;
    for (size_t i = 0; i < offset.size() - 1; i++)
        ReadProcessMemory(processHandle, (BYTE*)((int)value + offset[i]), &value, sizeof(value), NULL);

    char buffer[32];
    bool bReturn = ReadProcessMemory(processHandle, (BYTE*)((int)value + offset[offset.size() - 1]), buffer, 31, NULL);
    if (bReturn == 0) // error condition: no player name on the heap
        buffer[0] = { '\0' };

    return std::string(buffer);
};

uintptr_t GameHandler::getModuleBaseAddress(DWORD processId, TCHAR* moduleName)
{
    uintptr_t moduleBaseAddress = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId); // snapshot of all modules within process
    MODULEENTRY32 moduleEntry = { 0 };
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(snapshot, &moduleEntry)) { // store first module in moduleEntry
        do {
            if (_tcscmp(moduleEntry.szModule, moduleName) == 0) { // search for the module we are looking for
                moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
                break;
            }
        } while (Module32Next(snapshot, &moduleEntry)); // go through module entries and store in moduleEntry
    }
    CloseHandle(snapshot);

    return moduleBaseAddress;
};