#include "logger.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

static const char* objectiveNames[NUM_OBJECTIVES] = {
    "Master Key", "Clue 4", "Clue 2", "Clue 7",
    "Cabinet Unlocked", "Train Key", "Fuse Collected", "Fuse Installed",
    "Lift Pass", "Lift 2 Active", "Passenger Rescued", "Cat Carrier",
    "Cat Secured", "Train Prepared", "Mission Won", "Mission Failed"
};

bool saveGameStateToFile(const char* filename,
                         int playerFloor, int playerRoom,
                         int energy, int time, int score,
                         const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                         const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,
                         const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                         const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {
  std::ofstream outFile(filename);
  if (!outFile.is_open()) return false;

  outFile << "floor=" << playerFloor << "\nroom=" << playerRoom
          << "\nenergy=" << energy << "\ntime=" << time
          << "\nscore=" << score << "\ntrips=" << emergencyTripsLeft
          << "\ninventory=";
  for (int idx = 0; idx < inventoryCount && idx < MAX_INVENTORY_CAPACITY; ++idx) {
    outFile << inventory[idx];
  }
  outFile << "\n";

  for (int idx = 0; idx < NUM_OBJECTIVES; ++idx) {
    outFile << "objective" << idx << "=" << (objectives[idx] ? 1 : 0) << "\n";
  }

  for (int floorIdx = 0; floorIdx < NUM_FLOORS; ++floorIdx) {
    for (int roomIdx = 0; roomIdx < ROOMS_PER_FLOOR; ++roomIdx) {
      int count = entityCounts[floorIdx][roomIdx];
      if (count < 0) count = 0;
      if (count > MAX_ENTITIES_PER_ROOM) count = MAX_ENTITIES_PER_ROOM;
      outFile << "room" << floorIdx << "_" << roomIdx << "=" << count << ":";
      for (int slotIdx = 0; slotIdx < count; ++slotIdx) {
        outFile << roomEntities[floorIdx][roomIdx][slotIdx];
      }
      outFile << "\n";
    }
  }
  return static_cast<bool>(outFile);
}

bool loadGameStateFromFile(const char* filename,
                           int& playerFloor, int& playerRoom,
                           int& energy, int& time, int& score,
                           char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                           bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                           char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                           int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {
  std::ifstream inFile(filename);
  if (!inFile.is_open()) return false;

  int loadedFloor = -1, loadedRoom = -1, loadedEnergy = -1, loadedTime = -1;
  int loadedScore = 0, loadedTrips = 0, loadedInvCount = 0;
  char loadedInv[MAX_INVENTORY_CAPACITY] = {'\0'};
  bool loadedObj[NUM_OBJECTIVES] = {false};
  char loadedEnt[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM];
  int loadedCnt[NUM_FLOORS][ROOMS_PER_FLOOR] = {{0}};

  for (int f = 0; f < NUM_FLOORS; ++f)
    for (int r = 0; r < ROOMS_PER_FLOOR; ++r)
      for (int s = 0; s < MAX_ENTITIES_PER_ROOM; ++s)
        loadedEnt[f][r][s] = ENT_NONE;

  std::string line;
  while (std::getline(inFile, line)) {
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) continue;
    std::string key = line.substr(0, eqPos);
    std::string val = line.substr(eqPos + 1);
    int numVal = std::atoi(val.c_str());

    if (key == "floor") loadedFloor = numVal;
    else if (key == "room") loadedRoom = numVal;
    else if (key == "energy") loadedEnergy = numVal;
    else if (key == "time") loadedTime = numVal;
    else if (key == "score") loadedScore = numVal;
    else if (key == "trips") loadedTrips = numVal;
    else if (key == "inventory") {
      if (val.size() > static_cast<size_t>(MAX_INVENTORY_CAPACITY)) return false;
      loadedInvCount = 0;
      for (char ch : val) loadedInv[loadedInvCount++] = ch;
    } else if (key.rfind("objective", 0) == 0) {
      int objIdx = std::atoi(key.c_str() + 9);
      if (objIdx < 0 || objIdx >= NUM_OBJECTIVES) return false;
      loadedObj[objIdx] = (numVal != 0);
    } else if (key.rfind("room", 0) == 0) {
      int fIdx = -1, rIdx = -1;
      if (std::sscanf(key.c_str(), "room%d_%d", &fIdx, &rIdx) != 2 ||
          fIdx < 0 || fIdx >= NUM_FLOORS || rIdx < 0 || rIdx >= ROOMS_PER_FLOOR) {
        return false;
      }
      size_t colonPos = val.find(':');
      if (colonPos == std::string::npos) return false;
      int count = std::atoi(val.substr(0, colonPos).c_str());
      if (count < 0 || count > MAX_ENTITIES_PER_ROOM) return false;
      loadedCnt[fIdx][rIdx] = count;
      for (int slot = 0; slot < count; ++slot) {
        if (colonPos + 1 + static_cast<size_t>(slot) >= val.size()) return false;
        loadedEnt[fIdx][rIdx][slot] = val[colonPos + 1 + slot];
      }
    }
  }

  if (loadedFloor < 1 || loadedFloor > NUM_FLOORS ||
      loadedRoom < 1 || loadedRoom > ROOMS_PER_FLOOR ||
      loadedEnergy < 0 || loadedEnergy > MAX_ENERGY ||
      loadedTime < 0 || loadedTime > MAX_ENERGY ||
      loadedTrips < 0 || loadedTrips > 3 ||
      loadedScore < 0 || loadedScore > 520) {
    return false;
  }

  playerFloor = loadedFloor;
  playerRoom = loadedRoom;
  energy = loadedEnergy;
  time = loadedTime;
  score = loadedScore;
  emergencyTripsLeft = loadedTrips;
  inventoryCount = loadedInvCount;
  for (int idx = 0; idx < MAX_INVENTORY_CAPACITY; ++idx) inventory[idx] = loadedInv[idx];
  for (int idx = 0; idx < NUM_OBJECTIVES; ++idx) objectives[idx] = loadedObj[idx];
  for (int f = 0; f < NUM_FLOORS; ++f) {
    for (int r = 0; r < ROOMS_PER_FLOOR; ++r) {
      entityCounts[f][r] = loadedCnt[f][r];
      for (int s = 0; s < MAX_ENTITIES_PER_ROOM; ++s) {
        roomEntities[f][r][s] = loadedEnt[f][r][s];
      }
    }
  }
  return true;
}

void writeGameSummaryReport(const char* filename,
                            int score, int remainingEnergy, int remainingTime,
                            const bool objectives[NUM_OBJECTIVES], bool victory) {
  std::ofstream outFile(filename);
  if (!outFile.is_open()) return;

  outFile << "THE LAST TRAIN - " << (victory ? "VICTORY" : "DEFEAT / QUIT") << "\n"
          << "Score: " << score << " / 520\n"
          << "Energy: " << remainingEnergy << "\n"
          << "Time: " << remainingTime << "\n"
          << "Objectives:\n";
  for (int idx = 0; idx < NUM_OBJECTIVES; ++idx) {
    outFile << "[" << (objectives[idx] ? "X" : " ") << "] " << objectiveNames[idx] << "\n";
  }
}
