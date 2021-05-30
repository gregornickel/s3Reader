#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <iomanip>  // std::setw
#include <chrono>
#include <thread>
#include <fstream>  // std::ofstream
#include <tchar.h>  // _tcscmp
#include <vector>
#include "lib/nlohmann/json.hpp"

// for convenience
using json = nlohmann::json;

uintptr_t GetModuleBaseAddress(DWORD processId, TCHAR* moduleName) {
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);  // snapshot of all modules within process
	MODULEENTRY32 moduleEntry = { 0 };
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &moduleEntry))  // store first module in moduleEntry
	{
		do {
			if (_tcscmp(moduleEntry.szModule, moduleName) == 0)  // search for the module we are looking for
			{
				moduleBaseAddress = (uintptr_t)moduleEntry.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnapshot, &moduleEntry));  // go through module entries in snapshot and store in moduleEntry
	}
	CloseHandle(hSnapshot);

	return moduleBaseAddress;
}

int GetValue(HANDLE& processHandle, uintptr_t gameBaseAddress, unsigned int offset) {
	uintptr_t value = NULL;
	ReadProcessMemory(processHandle, (BYTE*)(gameBaseAddress + offset), &value, sizeof(value), NULL);

	return value;
}

int GetValue(HANDLE& processHandle, uintptr_t gameBaseAddress, std::vector<unsigned int> offset) {
	uintptr_t value = gameBaseAddress;
	for (size_t i = 0; i < offset.size(); i++) {
		ReadProcessMemory(processHandle, (BYTE*)((int)value +  offset[i]), &value, sizeof(value), NULL);
	}

	return value;
}

std::string GetName(HANDLE& processHandle, uintptr_t gameBaseAddress, std::vector<unsigned int> offset) {
	uintptr_t value = gameBaseAddress;
	for (size_t i = 0; i < offset.size() - 1; i++) {
		ReadProcessMemory(processHandle, (BYTE*)((int)value +  offset[i]), &value, sizeof(value), NULL);
	}

	char cValue[32];  // buffer of 32 characters
	bool bReturn = ReadProcessMemory(processHandle, (BYTE*)((int)value + offset[offset.size() - 1]), cValue, 31, NULL); 
	if (bReturn == 0) {
		// error condition: no player name on the heap 
		cValue[0] = { '\0' };
	}
	
	return std::string(cValue);
}

int main() {
	HWND hGameWindow = FindWindow(NULL, "S3");
	if (hGameWindow == NULL) {
		std::cout << "no game found, please start the game\n";
		std::cin.get();
		return 0;
	} 
	else {
		std::cout << "game found\n";
	}

	DWORD processId = NULL;  // ID of our Game
	GetWindowThreadProcessId(hGameWindow, &processId);
	HANDLE processHandle = NULL;
	processHandle = OpenProcess(PROCESS_VM_READ, FALSE, processId);
	if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) {
		std::cout << "failed to open process " << processId << " (processId) with processHandle " << processHandle << "\n";
		std::cin.get();
		return 0;
	}
	else {
		std::cout << "processHandle found\n";
	}

	char gameName[] = "s3_alobby.exe";
	uintptr_t gameBaseAddress = GetModuleBaseAddress(processId, _T(gameName));
	std::cout << "gameBaseAddress = " << std::hex << gameBaseAddress << std::dec << "\n";

	unsigned int offsetTick = 0x3DFD48;
	unsigned int offsetNumberOfPlayers = 0x3ACFA4;

	// initial stat offsets for player0, next player stats have an additional offset of +0x44 each
	std::vector<unsigned int> initialNameAddress{ 0x3ACFB4, 0x00 };  // two level pointer 
	std::vector<unsigned int> initialRaceAddress{ 0x3A7A78, 0x0064 };  // two level pointer 
	unsigned int initialTeamOffset = 0x3ACFC0;
	unsigned int initialSettlersOffset = 0x3ACFCC;
	unsigned int initialBuildingsOffset = 0x3ACFD0;
	unsigned int initialFoodOffset = 0x3ACFD4;
	unsigned int initialMinesOffset = 0x3ACFD8;
	unsigned int initialGoldOffset = 0x3ACFDC;
	unsigned int InitialMannaOffset = 0x3ACFE0;
	unsigned int initialSoldiersOffset = 0x3ACFE4;
	unsigned int initialBattlesOffset = 0x3ACFE8;

	// using strftime to display date and time 
	char dateBuffer[100];
	char timeBuffer[100];
	time_t rawtime;
	time(&rawtime);
	struct tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	std::strftime(dateBuffer, sizeof(dateBuffer), "%F", &timeinfo);
	std::strftime(timeBuffer, sizeof(timeBuffer), "%H-%M-%S", &timeinfo);

	auto t1 = std::chrono::high_resolution_clock::now();

	json j = {
		{"general", {
			{"date", dateBuffer},
			{"time", timeBuffer},
			{"map", "random (placeholder)"},
			{"rules", "standard (placeholder)"}
		}},
		{"stats", {
			{"entries", 0}
		}}
	};
	json jOverlay = j;

	int i = 0;
	bool gameEnded = false;

	while (i < 28800)  // loop while recording stats (about 4h)
	{
		i++;

		int tick = GetValue(processHandle, gameBaseAddress, offsetTick);

		if (tick > 0) {
			j["stats"]["tick"].push_back(tick);
			jOverlay["stats"]["tick"] = tick;

			// get constant values 
			int numberOfPlayers = GetValue(processHandle, gameBaseAddress, offsetNumberOfPlayers);
			j["general"]["numberOfPlayers"] = numberOfPlayers;
			jOverlay["general"]["numberOfPlayers"] = numberOfPlayers;

			// TODO: only record taken spots
			std::string previousPlayerName = "";

			for (size_t i = 0; i < 20; i++)  // iterate over all 20 player slots
			{
				std::string player = "player" + std::to_string(i);

				// additional offsets between players
				std::vector<unsigned int> nameOffset = { initialNameAddress[0] + (unsigned int)(0x44 * i), initialNameAddress[1]};
				std::vector<unsigned int> raceOffset = { initialRaceAddress[0], initialRaceAddress[1] + (unsigned int)(0xCC * i) };
				unsigned int statsOffset = 0x44 * i;

				std::string name = GetName(processHandle, gameBaseAddress, nameOffset);
				int race = GetValue(processHandle, gameBaseAddress, raceOffset);
				int team = GetValue(processHandle, gameBaseAddress, initialTeamOffset + statsOffset);
				int settlers = GetValue(processHandle, gameBaseAddress, initialSettlersOffset + statsOffset);
				int buildings = GetValue(processHandle, gameBaseAddress, initialBuildingsOffset + statsOffset);
				int food = GetValue(processHandle, gameBaseAddress, initialFoodOffset + statsOffset);
				int mines = GetValue(processHandle, gameBaseAddress, initialMinesOffset + statsOffset);
				int gold = GetValue(processHandle, gameBaseAddress, initialGoldOffset + statsOffset) / 2;
				int manna = GetValue(processHandle, gameBaseAddress, InitialMannaOffset + statsOffset);
				int soldiers = GetValue(processHandle, gameBaseAddress, initialSoldiersOffset + statsOffset);
				int battles = GetValue(processHandle, gameBaseAddress, initialBattlesOffset + statsOffset);
				int score = settlers*2 + buildings + food + mines + gold*2 + manna + soldiers*2 + battles*5;

				// note: this excludes multiple computer opponents
				if (previousPlayerName != name) {
					j["stats"][player]["name"] = name;
				} else {
					j["stats"][player]["name"] = "";
				}
				j["stats"][player]["race"] = race;
				j["stats"][player]["team"] = team;
				j["stats"][player]["settlers"].push_back(settlers);
				j["stats"][player]["buildings"].push_back(buildings);
				j["stats"][player]["food"].push_back(food);
				j["stats"][player]["mines"].push_back(mines);
				j["stats"][player]["gold"].push_back(gold);
				j["stats"][player]["manna"].push_back(manna);
				j["stats"][player]["soldiers"].push_back(soldiers);
				j["stats"][player]["battles"].push_back(battles);
				j["stats"][player]["score"].push_back(score);

				if (previousPlayerName != name) {
					jOverlay["stats"][player]["name"] = name;
				} else {
					jOverlay["stats"][player]["name"] = "";
				}
				jOverlay["stats"][player]["race"] = race;
				jOverlay["stats"][player]["team"] = team;
				jOverlay["stats"][player]["settlers"] = settlers;
				jOverlay["stats"][player]["buildings"] = buildings;
				jOverlay["stats"][player]["food"] = food;
				jOverlay["stats"][player]["mines"] = mines;
				jOverlay["stats"][player]["gold"] = gold;
				jOverlay["stats"][player]["manna"] = manna;
				jOverlay["stats"][player]["soldiers"] = soldiers;
				jOverlay["stats"][player]["battles"] = battles;
				jOverlay["stats"][player]["score"] = score;

				previousPlayerName = name;

				if ((int)j["stats"]["entries"] > 1) {
					if (gold < j["stats"][player]["gold"][(int)j["stats"]["entries"] - 1]) {
						gameEnded = 1;
					}
				}
			}

			// end recording if tick is not increasing for over 30 seconds 
			if (j["stats"]["entries"] > 60) {
				if (gameEnded || j["stats"]["tick"][(int)j["stats"]["entries"] - 60] == tick) {
					std::cout << "\nrecording ended" << std::endl;
					std::cin.get();
					break;
				}
			}

			auto t2 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
			auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

			std::cout << "\r(" << i << ") " << std::fixed << std::setprecision(1) << fp_ms.count() << " ms since start, tick " << tick << ", memory readout successful!";

			j["stats"]["time"].push_back(int_ms.count());
			j["stats"]["entries"] = (int)j["stats"]["entries"] + 1;
			jOverlay["general"]["gameEnded"] = gameEnded;

			// TODO: check and create 'Stats' folder if it doesn't exist

			// write JSON to file
			std::ofstream o("Stats/" + std::string(dateBuffer) + "_" + std::string(timeBuffer) + ".json");
			o << std::setw(4) << j << std::endl;

			std::ofstream oo("Stats/overlay-data.json");
			oo << std::setw(4) << jOverlay << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
