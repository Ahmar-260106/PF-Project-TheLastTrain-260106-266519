#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "physics.h"
#include "switches.h"
#include "util.h"
#include "viewer_iface.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void eraseEntity(char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                        int floorNum, int roomNum, int& entityCount, int removeIdx) {
  for (int idx = removeIdx; idx < entityCount - 1; ++idx) {
    roomEntities[floorNum - 1][roomNum - 1][idx] =
        roomEntities[floorNum - 1][roomNum - 1][idx + 1];
  }
  roomEntities[floorNum - 1][roomNum - 1][--entityCount] = ENT_NONE;
}

bool handleMoveCommand(int targetFloor, int targetRoom,
                       int& currentFloor, int& currentRoom,
                       int& energy, int& time,
                       const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                       const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                       int& emergencyTripsLeft, bool lift2Operational) {
  char connType = CONN_NONE;
  if (targetFloor < 1 || targetFloor > NUM_FLOORS ||
      targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR) {
    std::cout << "Move rejected: Target F" << targetFloor << "-R" << targetRoom << " is out of bounds.\n";
    return false;
  }

  if (!isDirectlyConnected(currentFloor, currentRoom, targetFloor, targetRoom,
                           horizontalConn, connType)) {
    if (currentFloor == targetFloor && std::abs(currentRoom - targetRoom) == 1) {
      int leftIdx = (currentRoom < targetRoom ? currentRoom : targetRoom) - 1;
      if (horizontalConn[currentFloor - 1][leftIdx] == CONN_SOLID_WALL) {
        std::cout << "Move rejected: Blocked by a solid wall (#).\n";
        return false;
      }
    }
    std::cout << "Move rejected: F" << targetFloor << "-R" << targetRoom
              << " is not directly connected to F" << currentFloor << "-R" << currentRoom << ".\n";
    return false;
  }

  if (!canTraverse(targetFloor, targetRoom, connType, inventory, inventoryCount,
                   emergencyTripsLeft, lift2Operational)) {
    if (connType == CONN_LOCKED_DOOR) {
      std::cout << "Move rejected: Locked door (L) requires Master Key (<).\n";
    } else if (connType == '2') {
      std::cout << "Move rejected: Lift 2 is inactive and 0 emergency trips remain.\n";
    } else {
      std::cout << "Move rejected: Passage cannot be traversed.\n";
    }
    return false;
  }

  int deltaEnergy = COST_MOVE_ORDINARY_ENERGY;
  int deltaTime = COST_MOVE_ORDINARY_TIME;
  if (connType == CONN_WINDOW) {
    deltaEnergy = COST_MOVE_WINDOW_ENERGY;
    deltaTime = COST_MOVE_WINDOW_TIME;
  } else if (connType == '1' || connType == '2') {
    deltaEnergy = COST_MOVE_LIFT_ENERGY;
    deltaTime = COST_MOVE_LIFT_TIME;
  }

  applyResourceDelta(energy, time, deltaEnergy, deltaTime);
  if (connType == '2' && !lift2Operational) {
    --emergencyTripsLeft;
  }
  currentFloor = targetFloor;
  currentRoom = targetRoom;
  return true;
}

bool handleInspectCommand(int currentFloor, int currentRoom,
                          int& energy, int& time,
                          const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int entityCount,
                          bool objectives[NUM_OBJECTIVES], int& score) {
  applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);
  for (int idx = 0; idx < entityCount; ++idx) {
    char symbol = roomEntities[currentFloor - 1][currentRoom - 1][idx];
    int objIndex = -1;
    if (symbol == ENT_CLUE_4) objIndex = OBJ_CLUE_4;
    else if (symbol == ENT_CLUE_2) objIndex = OBJ_CLUE_2;
    else if (symbol == ENT_CLUE_7) objIndex = OBJ_CLUE_7;

    if (objIndex >= 0 && !objectives[objIndex]) {
      objectives[objIndex] = true;
      score += SCORE_CLUE_FOUND;
    }
  }
  return true;
}

bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,
                          int& energy, int& time, int& score,
                          char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                          int& entityCount,
                          char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                          bool objectives[NUM_OBJECTIVES]) {
  int foundIdx = -1;
  for (int idx = 0; idx < entityCount; ++idx) {
    if (roomEntities[currentFloor - 1][currentRoom - 1][idx] == itemSymbol) {
      foundIdx = idx;
      break;
    }
  }
  if (foundIdx < 0) return false;

  if (itemSymbol == ENT_PASSENGER || itemSymbol == ENT_CAT ||
      itemSymbol == ENT_CONTROL_PANEL || itemSymbol == ENT_CABINET_TRAIN_KEY ||
      itemSymbol == ENT_CLUE_4 || itemSymbol == ENT_CLUE_2 || itemSymbol == ENT_CLUE_7) {
    std::cout << "That fixture cannot be collected.\n";
    return false;
  }

  if (itemSymbol == ENT_ENERGY_PACK ||
      itemSymbol == ENT_EMERGENCY_TOOLKIT ||
      itemSymbol == ENT_OBSERVATION_TOKEN) {
    applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
    if (itemSymbol == ENT_ENERGY_PACK) energy = clampInt(energy + BONUS_ENERGY_PACK, 0, MAX_ENERGY);
    else if (itemSymbol == ENT_EMERGENCY_TOOLKIT) energy = clampInt(energy + BONUS_EMERGENCY_TOOLKIT, 0, MAX_ENERGY);
    else if (itemSymbol == ENT_OBSERVATION_TOKEN) time = clampInt(time + BONUS_OBSERVATION_TOKEN, 0, MAX_ENERGY);
  } else {
    if (!addItem(inventory, inventoryCount, itemSymbol)) {
      std::cout << "Inventory is full.\n";
      return false;
    }
    applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
    if (itemSymbol == ENT_MASTER_KEY) { objectives[OBJ_MASTER_KEY] = true; score += SCORE_KEY_COLLECT; }
    else if (itemSymbol == ENT_FUSE) objectives[OBJ_FUSE_COLLECTED] = true;
    else if (itemSymbol == ENT_LIFT_PASS) {
      objectives[OBJ_LIFT_PASS] = true;
      score += SCORE_LIFT_PASS;
      if (objectives[OBJ_FUSE_INSTALLED]) objectives[OBJ_LIFT2_ACTIVE] = true;
    } else if (itemSymbol == ENT_CAT_CARRIER) objectives[OBJ_CAT_CARRIER] = true;
  }

  eraseEntity(roomEntities, currentFloor, currentRoom, entityCount, foundIdx);
  return true;
}

bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,
                      int& energy, int& time, int& score,
                      char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                      bool objectives[NUM_OBJECTIVES]) {
  if (!hasItem(inventory, inventoryCount, itemSymbol)) return false;

  if (itemSymbol == ENT_FUSE && currentFloor == 3 && currentRoom == 4) {
    if (!installFuseControlPanel(inventory, inventoryCount, objectives, score)) return false;
    if (objectives[OBJ_LIFT_PASS]) objectives[OBJ_LIFT2_ACTIVE] = true;
  } else if (itemSymbol == ENT_MEDICAL_KIT && currentFloor == 4 && currentRoom == 1) {
    removeItem(inventory, inventoryCount, itemSymbol);
    objectives[OBJ_PASSENGER_RESCUED] = true;
    score += SCORE_RESCUE_PASSENGER;
  } else if (itemSymbol == ENT_CAT_CARRIER && currentFloor == 3 && currentRoom == 3) {
    removeItem(inventory, inventoryCount, itemSymbol);
    objectives[OBJ_CAT_SECURED] = true;
    score += SCORE_SECURE_CAT;
  } else if (itemSymbol == ENT_CABINET_TRAIN_KEY && currentFloor == 5 && currentRoom == 2 &&
             objectives[OBJ_TRAIN_KEY] && objectives[OBJ_PASSENGER_RESCUED] &&
             objectives[OBJ_CAT_SECURED]) {
    applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
    score += SCORE_PREPARE_TRAIN + SCORE_MISSION_VICTORY;
    objectives[OBJ_TRAIN_PREPARED] = true;
    objectives[OBJ_MISSION_WON] = true;
    return true;
  } else {
    return false;
  }

  applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
  return true;
}

bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,
                                bool objectives[NUM_OBJECTIVES], int& score,
                                char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                                int& entityCount) {
  (void)roomEntities;
  (void)entityCount;
  if (currentFloor != 5 || currentRoom != 1 ||
      objectives[OBJ_CABINET_UNLOCKED] || !verifyCabinetCode(code, objectives)) {
    return false;
  }
  objectives[OBJ_CABINET_UNLOCKED] = true;
  objectives[OBJ_TRAIN_KEY] = true;
  score += SCORE_CABINET_UNLOCK + SCORE_TRAIN_KEY;
  return true;
}

bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES]) {
  return energy <= 0 || time <= 0 || objectives[OBJ_MISSION_WON] || objectives[OBJ_MISSION_FAILED];
}

void runSimulation(int& playerFloor, int& playerRoom,
                   int& energy, int& time, int& score,
                   char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,
                   bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
                   char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                   char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                   int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR]) {
  std::string line, auditLog;
  int stepCounter = 0;
  bool resumedFromSave = false;

  std::ifstream existingSave("out/savegame.txt");
  if (existingSave.good()) {
    existingSave.close();
    std::cout << "Saved game found. Resume? (y/n or enter starting floor 1-5): ";
    if (std::getline(std::cin, line)) {
      std::istringstream firstInput(line);
      std::string token;
      if (firstInput >> token) {
        if (token == "y" || token == "Y" || token == "yes") {
          if (loadGameStateFromFile("out/savegame.txt", playerFloor, playerRoom, energy, time,
                                    score, inventory, inventoryCount, objectives,
                                    emergencyTripsLeft, roomEntities, entityCounts)) {
            resumedFromSave = true;
            std::cout << "Resumed saved game at F" << playerFloor << "-R" << playerRoom << ".\n";
          } else {
            std::cout << "Saved file invalid. Starting new game.\n";
          }
        } else if (token != "n" && token != "N" && token != "no") {
          int startFloor = std::atoi(token.c_str()), startRoom = 0;
          if (!(firstInput >> startRoom)) {
            std::cout << "Starting room (1-4): ";
            if (std::getline(std::cin, line)) { std::istringstream rs(line); rs >> startRoom; }
          }
          if (validateStartingRoom(startFloor, startRoom, horizontalConn)) {
            playerFloor = startFloor; playerRoom = startRoom;
          } else {
            std::cout << "Invalid starting room (" << startFloor << ", " << startRoom << "). Defaulting to F1-R1.\n";
          }
          resumedFromSave = true;
        }
      }
    }
  }

  if (!resumedFromSave) {
    std::cout << "Starting floor (1-5): ";
    if (std::getline(std::cin, line)) {
      std::istringstream fs(line);
      int startFloor = 0, startRoom = 0;
      if (fs >> startFloor) {
        if (!(fs >> startRoom)) {
          std::cout << "Starting room (1-4): ";
          if (std::getline(std::cin, line)) { std::istringstream rs(line); rs >> startRoom; }
        }
        if (validateStartingRoom(startFloor, startRoom, horizontalConn)) {
          playerFloor = startFloor; playerRoom = startRoom;
        } else {
          std::cout << "Invalid starting room (" << startFloor << ", " << startRoom << "). Defaulting to F1-R1.\n";
        }
      } else {
        std::cout << "Invalid input. Defaulting to F1-R1.\n";
      }
    }
  }

  auditLog += "START -> F" + std::to_string(playerFloor) + "-R" + std::to_string(playerRoom) +
              " | E:" + std::to_string(energy) + " | T:" + std::to_string(time) +
              " | Score:" + std::to_string(score) + "\n";

  bool done = false, manualQuit = false;
  while (!done) {
    renderDashboard(playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, emergencyTripsLeft);
    std::cout << "> ";
    if (!std::getline(std::cin, line)) line = "quit";

    std::istringstream cmdStream(line);
    std::string cmd, arg;
    cmdStream >> cmd;

    if (cmd == "m" || cmd == "move") {
      int tf = 0, tr = 0;
      if (!(cmdStream >> tf >> tr)) std::cout << "Usage: move <floor 1-5> <room 1-4>\n";
      else handleMoveCommand(tf, tr, playerFloor, playerRoom, energy, time, horizontalConn,
                             inventory, inventoryCount, emergencyTripsLeft, objectives[OBJ_LIFT2_ACTIVE]);
    } else if (cmd == "i" || cmd == "inspect") {
      handleInspectCommand(playerFloor, playerRoom, energy, time, roomEntities,
                           entityCounts[playerFloor - 1][playerRoom - 1], objectives, score);
      renderRoomDetails(playerFloor, playerRoom, roomEntities, entityCounts[playerFloor - 1][playerRoom - 1]);
    } else if (cmd == "c" || cmd == "collect") {
      if (!(cmdStream >> arg) || arg.size() != 1) std::cout << "Usage: collect <single-symbol>\n";
      else {
        bool isCard = (arg[0] == ENT_EMERGENCY_CARD);
        if (!handleCollectCommand(arg[0], playerFloor, playerRoom, energy, time, score, roomEntities,
                                  entityCounts[playerFloor - 1][playerRoom - 1], inventory, inventoryCount, objectives))
          std::cout << "Item '" << arg[0] << "' unavailable or cannot be collected.\n";
        else {
          if (isCard) emergencyTripsLeft = 3;
          std::cout << "Collected " << getEntityName(arg[0]) << " (" << arg[0] << ").\n";
        }
      }
    } else if (cmd == "u" || cmd == "use") {
      if (!(cmdStream >> arg) || arg.size() != 1) std::cout << "Usage: use <single-symbol>\n";
      else if (!handleUseCommand(arg[0], playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, objectives))
        std::cout << "Cannot use '" << arg[0] << "' in this room.\n";
      else std::cout << "Used " << getEntityName(arg[0]) << " (" << arg[0] << ").\n";
    } else if (cmd == "unlock" || cmd == "code") {
      if (!(cmdStream >> arg)) std::cout << "Usage: unlock <3-digit-code>\n";
      else if (handleUnlockCabinetCommand(arg.c_str(), playerFloor, playerRoom, objectives, score,
                                          roomEntities, entityCounts[playerFloor - 1][playerRoom - 1])) {
        addItem(inventory, inventoryCount, ENT_CABINET_TRAIN_KEY);
        applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
        std::cout << "Cabinet unlocked! Acquired Train Key (^).\n";
      } else std::cout << "Code rejected or required clues missing.\n";
    } else if (cmd == "save") {
      if (saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                              inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts))
        std::cout << "Game saved to out/savegame.txt.\n";
      else std::cout << "Error: Failed to save game.\n";
    } else if (cmd == "load") {
      if (loadGameStateFromFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                                inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts))
        std::cout << "Game loaded from out/savegame.txt.\n";
      else std::cout << "Error: Failed to load valid savegame.\n";
    } else if (cmd == "map") renderAsciiBuilding(playerFloor, playerRoom);
    else if (cmd == "status") renderObjectivesChecklist(objectives);
    else if (cmd == "help")
      std::cout << "Commands: move/m F R | inspect/i | collect/c SYMBOL | use/u SYMBOL | unlock/code CODE | save | load | map | status | quit\n";
    else if (cmd == "quit") { manualQuit = true; done = true; }
    else if (!cmd.empty()) std::cout << "Unknown command. Type 'help' for valid commands.\n";

    if (!cmd.empty()) {
      ++stepCounter;
      auditLog += "Step " + std::to_string(stepCounter) + ": " + line +
                  " -> F" + std::to_string(playerFloor) + "-R" + std::to_string(playerRoom) +
                  " | E:" + std::to_string(energy) + " | T:" + std::to_string(time) +
                  " | Score:" + std::to_string(score) + "\n";
    }
    if (!objectives[OBJ_MISSION_WON] && (energy <= 0 || time <= 0)) { objectives[OBJ_MISSION_FAILED] = true; done = true; }
    if (objectives[OBJ_MISSION_WON]) done = true;
  }

  if (objectives[OBJ_MISSION_WON]) {
    std::cout << "\n===========================================\n"
              << " VICTORY! THE LAST TRAIN DEPARTED SAFELY! \n"
              << " Final Score: " << score << " / 520\n"
              << "===========================================\n";
  } else if (objectives[OBJ_MISSION_FAILED]) {
    std::cout << "\n===========================================\n"
              << " MISSION FAILED! Resources depleted.       \n"
              << " Final Score: " << score << " / 520\n"
              << "===========================================\n";
  } else std::cout << "\nSimulation ended by player.\n";

  if (!manualQuit) {
    saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score,
                        inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts);
  }
  writeGameSummaryReport("out/game_summary.txt", score, energy, time, objectives, objectives[OBJ_MISSION_WON]);

  std::ofstream summaryAppend("out/game_summary.txt", std::ios::app);
  if (summaryAppend.is_open()) summaryAppend << "\nSESSION AUDIT LOG (DETERMINISTIC TRACE):\n" << auditLog;
}
