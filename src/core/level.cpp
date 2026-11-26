#include "level.h"
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

const char* getEntityName(char entitySymbol) {
  switch (entitySymbol) {
    case '<': return "Master Key";
    case ')': return "Energy Pack";
    case '\\': return "Emergency Toolkit";
    case '4': return "Clue 4";
    case '*': return "Fuse";
    case '~': return "Cat";
    case ';': return "Control Panel";
    case '(': return "Lift Pass";
    case '!': return "Passenger";
    case '%': return "Medical Kit";
    case '$': return "Emergency Access Card";
    case '2': return "Clue 2";
    case '7': return "Clue 7";
    case '^': return "Train Key Cabinet";
    case '@': return "Cat Carrier";
    case '&': return "Observation Token";
    default: return "Unknown";
  }
}

static bool isValidEntitySymbol(char ch) {
  const char* validSymbols = "<)\\4*~;(!%$27^@&";
  for (int idx = 0; validSymbols[idx] != '\0'; ++idx) {
    if (validSymbols[idx] == ch) return true;
  }
  return false;
}

bool loadBuildingMap(const char* filename,
                     char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                     char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                     int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {
  std::ifstream inputFile(filename);
  if (!inputFile.is_open()) return false;

  for (int floorIdx = 0; floorIdx < NUM_FLOORS; ++floorIdx) {
    for (int connIdx = 0; connIdx < ROOMS_PER_FLOOR - 1; ++connIdx) {
      horizontalConn[floorIdx][connIdx] = CONN_NONE;
    }
    for (int roomIdx = 0; roomIdx < ROOMS_PER_FLOOR; ++roomIdx) {
      entityCounts[floorIdx][roomIdx] = 0;
      for (int slotIdx = 0; slotIdx < MAX_ENTITIES_PER_ROOM; ++slotIdx) {
        roomEntities[floorIdx][roomIdx][slotIdx] = ENT_NONE;
      }
    }
  }

  std::string line;
  int parsedFloors = 0;

  while (std::getline(inputFile, line)) {
    int floorNum = 0;
    size_t r1Pos = std::string::npos;
    for (int candidate = 1; candidate <= NUM_FLOORS; ++candidate) {
      std::string tag = "F" + std::to_string(candidate) + "-R1";
      size_t pos = line.find(tag);
      if (pos != std::string::npos) {
        floorNum = candidate;
        r1Pos = pos;
        break;
      }
    }
    if (floorNum == 0) continue;

    int floorIdx = floorNum - 1;
    size_t roomPositions[ROOMS_PER_FLOOR];
    bool allRoomsFound = true;
    for (int roomNum = 1; roomNum <= ROOMS_PER_FLOOR; ++roomNum) {
      std::string tag = "F" + std::to_string(floorNum) + "-R" + std::to_string(roomNum);
      roomPositions[roomNum - 1] = line.find(tag, r1Pos);
      if (roomPositions[roomNum - 1] == std::string::npos) {
        allRoomsFound = false;
        break;
      }
    }
    if (!allRoomsFound) continue;

    for (int connIdx = 0; connIdx < ROOMS_PER_FLOOR - 1; ++connIdx) {
      size_t startCol = roomPositions[connIdx] + 5;
      size_t endCol = roomPositions[connIdx + 1];
      char detectedConn = CONN_NONE;
      for (size_t col = startCol; col < endCol && col < line.size(); ++col) {
        char ch = line[col];
        if (ch == 'L' && col + 1 < line.size() && line[col + 1] == '1') {
          detectedConn = CONN_NONE;
          break;
        }
        if (ch == CONN_DOOR || ch == CONN_WINDOW || ch == CONN_LOCKED_DOOR || ch == CONN_SOLID_WALL) {
          detectedConn = ch;
          break;
        }
      }
      horizontalConn[floorIdx][connIdx] = detectedConn;
    }

    size_t gridStart = line.rfind('|', roomPositions[0]);
    if (gridStart == std::string::npos) gridStart = 0;
    std::string entityLine;
    if (std::getline(inputFile, entityLine)) {
      size_t bars[5];
      int barCount = 0;
      for (size_t col = gridStart; col < entityLine.size() && barCount < 5; ++col) {
        if (entityLine[col] == '|') bars[barCount++] = col;
      }
      if (barCount == 5) {
        for (int roomIdx = 0; roomIdx < ROOMS_PER_FLOOR; ++roomIdx) {
          size_t leftBar = bars[roomIdx] + 1;
          size_t rightBar = bars[roomIdx + 1];
          for (size_t col = leftBar; col < rightBar && col < entityLine.size(); ++col) {
            char symbol = entityLine[col];
            if (isValidEntitySymbol(symbol) &&
                entityCounts[floorIdx][roomIdx] < MAX_ENTITIES_PER_ROOM) {
              roomEntities[floorIdx][roomIdx][entityCounts[floorIdx][roomIdx]++] = symbol;
            }
          }
        }
      }
    }
    ++parsedFloors;
  }
  return parsedFloors == NUM_FLOORS;
}

bool loadInitialState(const char* filename,
                      int& playerFloor, int& playerRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft) {
  std::ifstream inputFile(filename);
  if (!inputFile.is_open()) return false;

  playerFloor = 1;
  playerRoom = 1;
  energy = INITIAL_ENERGY;
  time = INITIAL_TIME;
  score = INITIAL_SCORE;
  inventoryCount = 0;
  emergencyTripsLeft = 0;

  for (int idx = 0; idx < MAX_INVENTORY_CAPACITY; ++idx) inventory[idx] = '\0';
  for (int idx = 0; idx < NUM_OBJECTIVES; ++idx) objectives[idx] = false;

  std::string line;
  while (std::getline(inputFile, line)) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) continue;
    std::string key = line.substr(0, colonPos);
    std::string val = line.substr(colonPos + 1);
    while (!val.empty() && std::isspace(static_cast<unsigned char>(val.front()))) val.erase(val.begin());
    while (!val.empty() && std::isspace(static_cast<unsigned char>(val.back()))) val.pop_back();

    if (key == "INITIAL_ENERGY") energy = std::atoi(val.c_str());
    else if (key == "INITIAL_TIME") time = std::atoi(val.c_str());
    else if (key == "INITIAL_SCORE") score = std::atoi(val.c_str());
    else if (key == "PASSENGER_RESCUED") objectives[OBJ_PASSENGER_RESCUED] = (val == "YES");
    else if (key == "CAT_RESCUED") objectives[OBJ_CAT_SECURED] = (val == "YES");
    else if (key == "CABINET_UNLOCKED") objectives[OBJ_CABINET_UNLOCKED] = (val == "YES");
    else if (key == "TRAIN_KEY_COLLECTED") objectives[OBJ_TRAIN_KEY] = (val == "YES");
    else if (key == "FUSE_INSTALLED") objectives[OBJ_FUSE_INSTALLED] = (val == "YES");
    else if (key == "LIFT_PASS_COLLECTED") objectives[OBJ_LIFT_PASS] = (val == "YES");
    else if (key == "TRAIN_PREPARED") objectives[OBJ_TRAIN_PREPARED] = (val == "YES");
  }
  return true;
}

bool validateStartingRoom(int startFloor, int startRoom,
                          const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1]) {
  if (startFloor < 1 || startFloor > NUM_FLOORS ||
      startRoom < 1 || startRoom > ROOMS_PER_FLOOR) {
    return false;
  }

  bool visited[NUM_FLOORS][ROOMS_PER_FLOOR] = {{false}};
  int queueFloor[TOTAL_ROOMS];
  int queueRoom[TOTAL_ROOMS];
  int head = 0, tail = 0;

  visited[startFloor - 1][startRoom - 1] = true;
  queueFloor[tail] = startFloor;
  queueRoom[tail] = startRoom;
  ++tail;

  int reachableCount = 0;
  while (head < tail) {
    int currF = queueFloor[head];
    int currR = queueRoom[head];
    ++head;
    ++reachableCount;

    for (int nextF = 1; nextF <= NUM_FLOORS; ++nextF) {
      for (int nextR = 1; nextR <= ROOMS_PER_FLOOR; ++nextR) {
        if (visited[nextF - 1][nextR - 1]) continue;
        bool connected = false;

        if (currF == nextF && std::abs(currR - nextR) == 1) {
          char conn = horizontalConn[currF - 1][(currR < nextR ? currR : nextR) - 1];
          if (conn != CONN_NONE && conn != CONN_SOLID_WALL) connected = true;
        }
        if (currR == 1 && nextR == 1 &&
            ((currF == 1 && nextF == 2) || (currF == 2 && nextF == 1) ||
             (currF == 2 && nextF == 3) || (currF == 3 && nextF == 2) ||
             (currF == 4 && nextF == 5) || (currF == 5 && nextF == 4))) {
          connected = true;
        }
        if ((currF == 1 || currF == 2) && (nextF == 1 || nextF == 2) &&
            (currR == 2 || currR == 3) && (nextR == 2 || nextR == 3)) {
          connected = true;
        }
        if (currR == 4 && nextR == 4 &&
            (currF == 2 || currF == 4 || currF == 5) &&
            (nextF == 2 || nextF == 4 || nextF == 5)) {
          connected = true;
        }

        if (connected) {
          visited[nextF - 1][nextR - 1] = true;
          queueFloor[tail] = nextF;
          queueRoom[tail] = nextR;
          ++tail;
        }
      }
    }
  }
  return reachableCount == TOTAL_ROOMS;
}
