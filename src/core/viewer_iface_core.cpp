#include "level.h"
#include "viewer_iface.h"
#include <iostream>

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score,
                     const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                     int emergencyTripsLeft) {
  std::cout << "F" << playerFloor << "-R" << playerRoom
            << " | Energy " << energy << " | Time " << time
            << " | Score " << score << " | Inventory ";
  for (int idx = 0; idx < inventoryCount; ++idx) std::cout << inventory[idx] << ' ';
  std::cout << "| Lift 2 trips " << emergencyTripsLeft << "\n";
}

void renderRoomDetails(int floor, int room,
                       const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],
                       int entityCount) {
  std::cout << "Contents F" << floor << "-R" << room << ": ";
  if (entityCount == 0) std::cout << "(empty)";
  for (int idx = 0; idx < entityCount; ++idx) {
    char sym = roomEntities[floor - 1][room - 1][idx];
    std::cout << sym << " (" << getEntityName(sym) << ") ";
  }
  std::cout << "\n";
}

void renderAsciiBuilding(int playerFloor, int playerRoom) {
  const char* rowLinks[NUM_FLOORS][3] = {
      {"-W-", "L1 ", "-D-"}, {"-D-", "L1 ", "-D-"}, {"-L-", "-W-", "-D-"},
      {"-W-", "-#-", "-D-"}, {"-D-", "-D-", "-W-"}
  };
  const char* leftStairs[NUM_FLOORS] = {"S1 ", "S12", "S2 ", "S3 ", "S3 "};
  const char* rightLift[NUM_FLOORS] = {"   ", " L2", " | ", " L2", " L2"};

  std::cout << "BUILDING MAP (D=Door, W=Window, L=Locked, #=Wall, S1-S3=Stairs, L1/L2=Lifts, P=Player)\n";
  for (int floorNum = NUM_FLOORS; floorNum >= 1; --floorNum) {
    std::cout << leftStairs[floorNum - 1] << " ";
    for (int roomNum = 1; roomNum <= ROOMS_PER_FLOOR; ++roomNum) {
      std::cout << "[" << (playerFloor == floorNum && playerRoom == roomNum ? "P" : " ")
                << " F" << floorNum << "-R" << roomNum << "]";
      if (roomNum < ROOMS_PER_FLOOR) std::cout << rowLinks[floorNum - 1][roomNum - 1];
    }
    std::cout << rightLift[floorNum - 1] << "\n";
  }
}

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES]) {
  const char* names[NUM_OBJECTIVES] = {
      "Master Key", "Clue 4", "Clue 2", "Clue 7",
      "Cabinet Unlocked", "Train Key", "Fuse Collected", "Fuse Installed",
      "Lift Pass", "Lift 2 Active", "Passenger Rescued", "Cat Carrier",
      "Cat Secured", "Train Prepared", "Mission Won", "Mission Failed"
  };
  std::cout << "Objectives: ";
  for (int idx = 0; idx < NUM_OBJECTIVES; ++idx) std::cout << "[" << (objectives[idx] ? 'X' : ' ') << "]" << names[idx] << " ";
  std::cout << "\n";
}
