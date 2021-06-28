#include <iostream>
#include <fstream> // std::ofstream
#include <iomanip> // std::setw
#include <chrono>
#include <thread>
#include <vector>
#include "lib/nlohmann/json.hpp"
#include "GameHandler.h"

// for convenience
using json = nlohmann::json;

int main(int argc, char* argv[])
{
    std::string gameName = "s3_alobby.exe";
    std::string windowName = "S3";
    GameHandler s3 = GameHandler(gameName, windowName);
    s3.find();

    unsigned int offsetTick = 0x3DFD48;
    unsigned int offsetNumberOfPlayers = 0x3ACFA4;

    // initial stat offsets for player0, next player stats have an additional offset of +0x44 each
    std::vector<unsigned int> initialNameAddress{ 0x3ACFB4, 0x00 }; // two level pointer
    std::vector<unsigned int> initialRaceAddress{ 0x3A7A78, 0x64 }; // two level pointer
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

    // clang-format off
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
    // clang-format on

    bool gameEnded = false;

    while (!gameEnded) {
        int tick = s3.readInt(offsetTick);

        if (tick > 0) {
            j["stats"]["tick"].push_back(tick);
            jOverlay["stats"]["tick"] = tick;

            // get constant values
            int numberOfPlayers = s3.readInt(offsetNumberOfPlayers);
            j["general"]["numberOfPlayers"] = numberOfPlayers;
            jOverlay["general"]["numberOfPlayers"] = numberOfPlayers;

            // TODO: only record taken spots
            std::string previousPlayerName = "";

            for (size_t i = 0; i < 20; i++) { // iterate over all 20 player slots
                std::string player = "player" + std::to_string(i);

                // additional offsets between players
                std::vector<unsigned int> nameOffset = { initialNameAddress[0] + (unsigned int)(0x44 * i),
                                                         initialNameAddress[1] };
                std::vector<unsigned int> raceOffset = { initialRaceAddress[0],
                                                         initialRaceAddress[1] + (unsigned int)(0xCC * i) };
                unsigned int statsOffset = 0x44 * i;

                std::string name = s3.readString(nameOffset);
                int race = s3.readInt(raceOffset);
                int team = s3.readInt(initialTeamOffset + statsOffset);
                int settlers = s3.readInt(initialSettlersOffset + statsOffset);
                int buildings = s3.readInt(initialBuildingsOffset + statsOffset);
                int food = s3.readInt(initialFoodOffset + statsOffset);
                int mines = s3.readInt(initialMinesOffset + statsOffset);
                int gold = s3.readInt(initialGoldOffset + statsOffset) / 2;
                int manna = s3.readInt(InitialMannaOffset + statsOffset);
                int soldiers = s3.readInt(initialSoldiersOffset + statsOffset);
                int battles = s3.readInt(initialBattlesOffset + statsOffset);
                int score = settlers * 2 + buildings + food + mines + gold * 2 + manna + soldiers * 2 + battles * 5;

                // note: this excludes multiple computer opponents
                if (previousPlayerName != name)
                    j["stats"][player]["name"] = name;
                else
                    j["stats"][player]["name"] = "";
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

                if (previousPlayerName != name)
                    jOverlay["stats"][player]["name"] = name;
                else
                    jOverlay["stats"][player]["name"] = "";
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
                    if (gold < j["stats"][player]["gold"][(int)j["stats"]["entries"] - 1])
                        gameEnded = 1;
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

            std::cout << "\r(" << j["stats"]["entries"] << ") " << std::fixed << std::setprecision(1) << fp_ms.count()
                      << " ms since start, tick " << tick << ", memory readout successful!";

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
