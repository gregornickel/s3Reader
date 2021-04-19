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

DWORD GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
	DWORD dwModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);  // make snapshot of all modules within process
	MODULEENTRY32 ModuleEntry32 = { 0 };
	ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &ModuleEntry32))  //store first Module in ModuleEntry32
	{
		do {
			if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0)  // if Found Module matches Module we look for -> done!
			{
				dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnapshot, &ModuleEntry32));  // go through Module entries in Snapshot and store in ModuleEntry32
	}
	CloseHandle(hSnapshot);

	return dwModuleBaseAddress;
}

DWORD GetValue(HANDLE& processHandle, DWORD gameBaseAddress, DWORD offset) {
	DWORD value = NULL;
	ReadProcessMemory(processHandle, (LPVOID)(gameBaseAddress + offset), &value, sizeof(value), NULL);

	return value;
}

DWORD GetValue(HANDLE& processHandle, DWORD gameBaseAddress, std::vector<DWORD> offset) {
	DWORD value = NULL;
	ReadProcessMemory(processHandle, (LPVOID)(gameBaseAddress + offset.front()), &value, sizeof(value), NULL);
	for (size_t i = 1; i < offset.size(); i++) {
		ReadProcessMemory(processHandle, (LPVOID)(value + offset[i]), &value, sizeof(value), NULL);
	}

	return value;
}


int main() {
	HWND hGameWindow = FindWindow(NULL, "S3");
	if (hGameWindow == NULL) {
		std::cout << "No game found, please start the game! \n";
		std::cin.get();
		return 0;
	} 
	else {
		std::cout << "game found \n";
	}

	DWORD pID = NULL;  // ID of our Game
	GetWindowThreadProcessId(hGameWindow, &pID);
	HANDLE processHandle = NULL;
	processHandle = OpenProcess(PROCESS_VM_READ, FALSE, pID);
	if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) {
		std::cout << "failed to open process " << pID << " (pID) with processHandle " << processHandle << "\n";
		std::cin.get();
		return 0;
	}
	else {
		std::cout << "processHandle found" << "\n";
	}

	char gameName[] = "s3_alobby.exe";
	DWORD gameBaseAddress = GetModuleBaseAddress(_T(gameName), pID);
	std::cout << "gameBaseAddress = " << std::hex << gameBaseAddress << std::dec << "\n";

	DWORD offsetNumberOfPlayers = 0x3ACFA4;

	// initial stat offsets for player0, next player stats have an additional offset of +0x44 each
	DWORD initialTeamOffset = 0x3ACFC0;
	std::vector<DWORD> initialRaceAddress{ 0x3A7A78, 0x0064 };  // two level pointer 
	DWORD initialSettlersOffset = 0x3ACFCC;
	DWORD initialBuildingsOffset = 0x3ACFD0;
	DWORD initialFoodOffset = 0x3ACFD4;
	DWORD initialMinesOffset = 0x3ACFD8;
	DWORD initialGoldOffset = 0x3ACFDC;
	DWORD InitialMannaOffset = 0x3ACFE0;
	DWORD initialSoldiersOffset = 0x3ACFE4;
	DWORD initialBattlesOffset = 0x3ACFE8;

	// get constant values 
	DWORD numberOfPlayers = GetValue(processHandle, gameBaseAddress, offsetNumberOfPlayers);

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
			{"numberOfPlayers", numberOfPlayers},
			{"map", "random (placeholder)"},
			{"rules", "standard (placeholder)"}
		}},
		{"stats", {
			{"entries", 0}
		}}
	};

	int i = 0;
	bool gameEnded = false;

	while (i < 36000)  // loop while recording stats (about 2h)
	{
		i++;

		for (size_t i = 0; i < 20; i++)  // iterate over all 20 player slots
		{
			std::string player = "player" + std::to_string(i);

			// additional offsets between players
			DWORD stats_offset = 0x44 * i;
			std::vector<DWORD> race_offset = { initialRaceAddress[0], initialRaceAddress[1] + (DWORD)(0xCC * i) };

			DWORD team = GetValue(processHandle, gameBaseAddress, initialTeamOffset + stats_offset);
			DWORD race = GetValue(processHandle, gameBaseAddress, race_offset);
			// TODO: player names
			DWORD settlers = GetValue(processHandle, gameBaseAddress, initialSettlersOffset + stats_offset);
			DWORD buildings = GetValue(processHandle, gameBaseAddress, initialBuildingsOffset + stats_offset);
			DWORD food = GetValue(processHandle, gameBaseAddress, initialFoodOffset + stats_offset);
			DWORD mines = GetValue(processHandle, gameBaseAddress, initialMinesOffset + stats_offset);
			DWORD gold = GetValue(processHandle, gameBaseAddress, initialGoldOffset + stats_offset);
			DWORD manna = GetValue(processHandle, gameBaseAddress, InitialMannaOffset + stats_offset);
			DWORD soldiers = GetValue(processHandle, gameBaseAddress, initialSoldiersOffset + stats_offset);
			DWORD battles = GetValue(processHandle, gameBaseAddress, initialBattlesOffset + stats_offset);
			DWORD score = settlers*2 + buildings + food + mines + gold*2 + manna + soldiers*2 + battles*5;

			DWORD entries = j["stats"]["entries"];
			// if (j["stats"]["entries"] > 0 && score < j["stats"][player]["score"][entries - 1]) {
			// TODO: occasionally the values drop which should not happen and the recording stops, workaround: 
			if (j["stats"]["entries"] > 0 && score < j["stats"][player]["score"][entries - 1] && score==0) {
				gameEnded = true;
				std::cout << "\ndebug: break\n";
				std::cout << "debug: " << player << ", entries: " << entries << ", score: " << j["stats"][player]["score"][entries - 1] << " (i-1), " << score << "(i)\n";
				break;
			}

			j["stats"][player]["team"] = team;
			j["stats"][player]["race"] = race;
			j["stats"][player]["settlers"].push_back(settlers);
			j["stats"][player]["buildings"].push_back(buildings);
			j["stats"][player]["food"].push_back(food);
			j["stats"][player]["mines"].push_back(mines);
			j["stats"][player]["gold"].push_back(gold);
			j["stats"][player]["manna"].push_back(manna);
			j["stats"][player]["soldiers"].push_back(soldiers);
			j["stats"][player]["battles"].push_back(battles);
			j["stats"][player]["score"].push_back(score);
		}

		if (gameEnded) {
			std::cout << "recording ended" << std::endl;
			std::cin.get();
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		auto t2 = std::chrono::high_resolution_clock::now();

		// floating-point duration: no duration_cast needed
		std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

		// integral duration: requires duration_cast
		auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

		std::cout << "\r(" << i << ") " << std::fixed << std::setprecision(1) << fp_ms.count() << " ms since start, memory readout successful!";

		j["stats"]["time"].push_back(int_ms.count());
		j["stats"]["entries"] = (int)j["stats"]["entries"] + 1;

		// write JSON to file
		std::ofstream o("s3-" + std::string(dateBuffer) + "-" + std::string(timeBuffer) + ".json");
		o << std::setw(4) << j << std::endl;
	}
}
