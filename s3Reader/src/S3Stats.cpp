#include "S3Stats.hpp"

#define ROMAN 0;
#define EGYPT 1;
#define ASIAN 2;
#define AMAZON 3;

static int offsetTick = 0x3DFD48;
static int offsetNumPlayers = 0x3ACFA4;
static std::vector<int> offsetGoods{ 0x3A8244, 0x5E0 }; // two level pointer
static std::vector<int> offsetMap{ 0x3A7A48, 0x0 };
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

//           | Roman         | Egypt         | Asian         | Amazon
// 0x119448C | Convert       | Punish        | Call Help     | Cursed Arrows
// 0x1194488 | Vanish        | Bann          | Samurai Sword | Freeze
// 0x1194484 | Midas Touch   | Strengthen    | Shield        | Gold to Stone
// 0x1194480 | Gifts         | Gifts         | Stone to Iron | Gifts
// 0x119447C | Fear          | Forest Fire   | Gifts         | Send Goods
// 0x1194478 | Growth        | Fish to Meat  | Flood of Fish | Call Goods
// 0x1194474 | Grassland     | Siphon Swamp  | Stone Curse   | Forest Favor
// 0x1194470 | Eye           | Horus Heat    | Melt Snow     | Reveal Map
static std::vector<int> offsetMannaSpell{ 0x3E3408, 0x119448C };
static std::vector<int> offsetMannaAvailable{ 0x3E3408, 0x1194468 };

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
    // clang-format on

    filename = std::string(dateBuffer) + "_" + std::string(timeBuffer);

    t1 = std::chrono::high_resolution_clock::now();
}

bool S3Stats::record()
{
    bool gameEnded = false;
    int tick = s3.readInt(offsetTick);

    if (tick > 0) {
        // read constant values only ones
        if (j["stats"]["entries"] == 0) {
            getTakenSpots();

            j["general"]["numPlayers"] = s3.readInt(offsetNumPlayers);
            j["general"]["goods"] = s3.readInt(offsetGoods);
            j["general"]["map"] = s3.readString(offsetMap);

            for (int i : takenSpots) {
                readRace(i);
                std::vector<int> nameOffset = { offsetStatsName[0] + (int)(addOffsetStats * i), offsetStatsName[1] };
                j["stats"]["player" + std::to_string(i)]["name"] = s3.readString(nameOffset);
                j["stats"]["player" + std::to_string(i)]["team"] = s3.readInt(offsetStatsTeam + addOffsetStats * i);
            }

            jCast = j;
        }

        j["stats"]["tick"].push_back(tick);
        jCast["stats"]["tick"] = tick;
        jCast["general"]["gameWon"] = s3.readInt(offsetWin);

        for (int i : takenSpots) {
            readStats(i);
            readSpellCost(i);

            // gold drops if game is left
            std::string player = "player" + std::to_string(i);
            int numEntries = (int)j["stats"]["entries"];
            if (numEntries > 1 && j["stats"][player]["gold"][numEntries] < j["stats"][player]["gold"][numEntries - 1])
                gameEnded = 1;
        }

        // end recording if tick is not increasing for over 30 seconds
        int numEntries = (int)j["stats"]["entries"];
        if (gameEnded || (numEntries > 60 && j["stats"]["tick"][numEntries - 60] == tick)) {
            std::cout << "\nrecording ended" << std::endl;
            std::cin.get();
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
}

void S3Stats::save()
{
    // TODO: check and create 'Stats' folder if it doesn't exist

    // write JSON to file
    std::ofstream o("Stats/" + filename + ".json");
    o << std::setw(4) << j << std::endl;

    std::ofstream oo("Stats/overlay-data.json");
    oo << std::setw(4) << jCast << std::endl;
}

std::string S3Stats::overlayData() { return jCast.dump(); }

void S3Stats::getTakenSpots()
{
    for (size_t i = 0; i < 20; i++) {
        std::vector<int> spellOffset = { offsetMannaSpell[0], offsetMannaSpell[1] + (int)(addOffsetManna * i) };
        if (s3.readInt(spellOffset) != 1)
            takenSpots.push_back(i);
    }
}

int S3Stats::findRandomRace(int i)
{
    std::vector<int> spell1Offset = { offsetMannaSpell[0], offsetMannaSpell[1] + (int)(addOffsetManna * i) };
    int spell1Cost = s3.readInt(spell1Offset);
    switch (spell1Cost) {
    case 60:
        return ROMAN;
    case 15:
        return EGYPT;
    case 10:
        return ASIAN;
    case 20:
        return AMAZON;
    default:
        return -1;
    }
}

void S3Stats::readRace(int i)
{
    std::string player = "player" + std::to_string(i);
    std::vector<int> raceOffset = { offsetRace[0], offsetRace[1] + (int)(addOffsetRace * i) };
    int race = s3.readInt(raceOffset);

    if (race == 255) {
        j["stats"][player]["randomRace"] = 1;
        j["stats"][player]["race"] = findRandomRace(i);
    } else {
        j["stats"][player]["randomRace"] = 0;
        j["stats"][player]["race"] = race;
    }
}

void S3Stats::readStats(int i)
{
    std::string player = "player" + std::to_string(i);
    int settlers = s3.readInt(offsetStatsSettlers + addOffsetStats * i);
    int buildings = s3.readInt(offsetStatsBuildings + addOffsetStats * i);
    int food = s3.readInt(offsetStatsFood + addOffsetStats * i);
    int mines = s3.readInt(offsetStatsMines + addOffsetStats * i);
    int gold = s3.readInt(offsetStatsGold + addOffsetStats * i) / 2;
    int manna = s3.readInt(offsetStatsManna + addOffsetStats * i);
    int soldiers = s3.readInt(offsetStatsSoldiers + addOffsetStats * i);
    int battles = s3.readInt(offsetStatsBattles + addOffsetStats * i);
    int score = settlers * 2 + buildings + food + mines + gold * 2 + manna + soldiers * 2 + battles * 5;

    j["stats"][player]["settlers"].push_back(settlers);
    j["stats"][player]["buildings"].push_back(buildings);
    j["stats"][player]["food"].push_back(food);
    j["stats"][player]["mines"].push_back(mines);
    j["stats"][player]["gold"].push_back(gold);
    j["stats"][player]["manna"].push_back(manna);
    j["stats"][player]["soldiers"].push_back(soldiers);
    j["stats"][player]["battles"].push_back(battles);
    j["stats"][player]["score"].push_back(score);

    jCast["stats"][player]["settlers"] = settlers;
    jCast["stats"][player]["buildings"] = buildings;
    jCast["stats"][player]["food"] = food;
    jCast["stats"][player]["mines"] = mines;
    jCast["stats"][player]["gold"] = gold;
    jCast["stats"][player]["manna"] = manna;
    jCast["stats"][player]["soldiers"] = soldiers;
    jCast["stats"][player]["battles"] = battles;
    jCast["stats"][player]["score"] = score;
}

void S3Stats::readSpellCost(int i)
{
    std::string player = "player" + std::to_string(i);
    std::vector<int> mannaOffset = { offsetMannaAvailable[0], offsetMannaAvailable[1] + (int)(addOffsetManna * i) };
    jCast["stats"][player]["mannaAvailable"] = s3.readInt(mannaOffset);

    for (size_t k = 0; k < 8; k++) {
        std::vector<int> spellOffset = { offsetMannaSpell[0], offsetMannaSpell[1] + (int)(addOffsetManna * i - k * 4) };
        jCast["stats"][player]["spell"][k] = s3.readInt(spellOffset);
    }
}