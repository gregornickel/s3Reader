#ifndef S3READER_GAMEHANDLER_HPP_
#define S3READER_GAMEHANDLER_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h> // _tcscmp

class GameHandler
{
public:
    GameHandler();
    GameHandler(std::string gameName, std::string windowName);
    bool find();
    int readInt(int offset);
    int readInt(std::vector<int> offset);
    std::string readString(std::vector<int> offset);

private:
    uintptr_t getModuleBaseAddress(DWORD processId, TCHAR* moduleName);

    std::string gameName;
    std::string windowName;
    HANDLE processHandle;
    uintptr_t gameBaseAddress;
};

#endif // S3READER_GAMEHANDLER_HPP_