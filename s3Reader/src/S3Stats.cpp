#include "S3Stats.hpp"

static int offsetTick = 0x3DFD48;
static int offsetNumPlayers = 0x3ACFA4;
static std::vector<int> offsetGoods{ 0x3A8244, 0x5E0 }; // two level pointer
static std::vector<int> offsetMap{ 0x3A7A48, 0x0 };
static int offsetMapVisible = 0x3DFD84;
static int offsetWin = 0x730F64;

// initial offsets for player0, next players have additional offsets
static std::vector<int> offsetRace{ 0x3A7A78, 0x64 };
static std::vector<int> offsetStatsName{ 0x3ACFB4, 0x0 };
static int offsetStatsTeam = 0x3ACFC0;
static int offsetStatsSettlers = 0x3ACFCC;
static int offsetStatsBuildings = 0x3ACFD0;
static int offsetStatsFood = 0x3ACFD4;
static int offsetStatsMines = 0x3ACFD8;
static int offsetStatsGold = 0x3ACFDC;
static int offsetStatsManna = 0x3ACFE0;
static int offsetStatsSoldiers = 0x3ACFE4;
static int offsetStatsBattles = 0x3ACFE8;

static std::vector<int> offsetMannaCurrent{ 0x3E3408, 0x1194468 };
//        | Roman         | Egypt         | Asian         | Amazon
// Spell1 | Convert       | Punish        | Call Help     | Cursed Arrows
// Spell2 | Vanish        | Bann          | Samurai Sword | Freeze
// Spell3 | Midas Touch   | Strengthen    | Shield        | Gold to Stone
// Spell4 | Gifts         | Gifts         | Stone to Iron | Gifts
// Spell5 | Fear          | Forest Fire   | Gifts         | Send Goods
// Spell6 | Growth        | Fish to Meat  | Flood of Fish | Call Goods
// Spell7 | Grassland     | Siphon Swamp  | Stone Curse   | Forest Favor
// Spell8 | Eye           | Horus Heat    | Melt Snow     | Reveal Map
static std::vector<int> offsetMannaSpell1{ 0x3E3408, 0x119448C };
static std::vector<int> offsetMannaSpell2{ 0x3E3408, 0x1194488 };
static std::vector<int> offsetMannaSpell3{ 0x3E3408, 0x1194484 };
static std::vector<int> offsetMannaSpell4{ 0x3E3408, 0x1194480 };
static std::vector<int> offsetMannaSpell5{ 0x3E3408, 0x119447C };
static std::vector<int> offsetMannaSpell6{ 0x3E3408, 0x1194478 };
static std::vector<int> offsetMannaSpell7{ 0x3E3408, 0x1194474 };
static std::vector<int> offsetMannaSpell8{ 0x3E3408, 0x1194470 };

// additional offsets
static int addOffsetRace = 0xCC;
static int addOffsetStats = 0x44;
static int addOffsetManna = 0x148;

S3Stats::S3Stats(GameHandler& s3) : s3{ s3 }
{
    time_t rawtime;
    time(&rawtime);
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);

    char dateBuffer[100];
    char timeBuffer[100];
    std::strftime(dateBuffer, sizeof(dateBuffer), "%F", &timeinfo);
    std::strftime(timeBuffer, sizeof(timeBuffer), "%H-%M-%S", &timeinfo);

    // clang-format off
    j = {
        {"general", {
            {"date", dateBuffer},
            {"time", timeBuffer}
        }},
        {"stats", {
            {"entries", 0}
        }}
    };
    jCast = j;
    // clang-format on

    filename = std::string(dateBuffer) + "_" + std::string(timeBuffer);

    t1 = std::chrono::high_resolution_clock::now();
};

bool S3Stats::record()
{
    bool gameEnded = false;
    int tick = s3.readInt(offsetTick);

    if (tick > 0) {
        j["stats"]["tick"].push_back(tick);
        jCast["stats"]["tick"] = tick;

        // get constant values
        int numPlayers = s3.readInt(offsetNumPlayers);
        j["general"]["numPlayers"] = numPlayers;
        jCast["general"]["numPlayers"] = numPlayers;

        // TODO: only record taken spots
        std::string previousPlayerName = "";

        for (size_t i = 0; i < 20; i++) { // iterate over all 20 player slots
            std::string player = "player" + std::to_string(i);

            // additional offsets between players
            std::vector<int> nameOffset = { offsetStatsName[0] + (int)(addOffsetStats * i), offsetStatsName[1] };
            std::vector<int> raceOffset = { offsetRace[0], offsetRace[1] + (int)(addOffsetRace * i) };
            int addOffset = addOffsetStats * i;

            std::string name = s3.readString(nameOffset);
            int race = s3.readInt(raceOffset);
            int team = s3.readInt(offsetStatsTeam + addOffset);
            int settlers = s3.readInt(offsetStatsSettlers + addOffset);
            int buildings = s3.readInt(offsetStatsBuildings + addOffset);
            int food = s3.readInt(offsetStatsFood + addOffset);
            int mines = s3.readInt(offsetStatsMines + addOffset);
            int gold = s3.readInt(offsetStatsGold + addOffset) / 2;
            int manna = s3.readInt(offsetStatsManna + addOffset);
            int soldiers = s3.readInt(offsetStatsSoldiers + addOffset);
            int battles = s3.readInt(offsetStatsBattles + addOffset);
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
                jCast["stats"][player]["name"] = name;
            else
                jCast["stats"][player]["name"] = "";
            jCast["stats"][player]["race"] = race;
            jCast["stats"][player]["team"] = team;
            jCast["stats"][player]["settlers"] = settlers;
            jCast["stats"][player]["buildings"] = buildings;
            jCast["stats"][player]["food"] = food;
            jCast["stats"][player]["mines"] = mines;
            jCast["stats"][player]["gold"] = gold;
            jCast["stats"][player]["manna"] = manna;
            jCast["stats"][player]["soldiers"] = soldiers;
            jCast["stats"][player]["battles"] = battles;
            jCast["stats"][player]["score"] = score;

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
                // break;
            }
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

        std::cout << "\r(" << j["stats"]["entries"] << ") " << std::fixed << std::setprecision(1) << fp_ms.count()
                  << " ms since start, tick " << tick << ", memory readout successful!";

        j["stats"]["time"].push_back(int_ms.count());
        j["stats"]["entries"] = (int)j["stats"]["entries"] + 1;
        jCast["general"]["gameEnded"] = gameEnded;
    }
    return gameEnded;
};

void S3Stats::save()
{
    // TODO: check and create 'Stats' folder if it doesn't exist

    // write JSON to file
    std::ofstream o("Stats/" + filename + ".json");
    o << std::setw(4) << j << std::endl;

    std::ofstream oo("Stats/overlay-data.json");
    oo << std::setw(4) << jCast << std::endl;
};
