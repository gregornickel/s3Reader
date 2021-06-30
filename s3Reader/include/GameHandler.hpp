#ifndef S3READER_GAMEHANDLER_H_
#define S3READER_GAMEHANDLER_H_

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
    std::string m_gameName;
    std::string m_windowName;
    HANDLE m_processHandle;
    uintptr_t m_gameBaseAddress;

    uintptr_t getModuleBaseAddress(DWORD processId, TCHAR* moduleName);
};

#endif // S3READER_GAMEHANDLER_H_