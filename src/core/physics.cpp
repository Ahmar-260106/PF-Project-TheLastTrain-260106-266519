#include "physics.h"
#include "switches.h"
#include "util.h"
#include <cstdlib>

bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol) {
  for (int idx = 0; idx < inventoryCount; ++idx) {
    if (inventory[idx] == itemSymbol) return true;
  }
  return false;
}

bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol) {
  if (hasItem(inventory, inventoryCount, itemSymbol)) return true;
  if (inventoryCount >= MAX_INVENTORY_CAPACITY) return false;
  inventory[inventoryCount++] = itemSymbol;
  return true;
}

bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol) {
  for (int idx = 0; idx < inventoryCount; ++idx) {
    if (inventory[idx] == itemSymbol) {
      for (int shift = idx; shift < inventoryCount - 1; ++shift) {
        inventory[shift] = inventory[shift + 1];
      }
      inventory[--inventoryCount] = '\0';
      return true;
    }
  }
  return false;
}

bool isDirectlyConnected(int fromF, int fromR, int toF, int toR,
                         const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char& outConnType) {
  outConnType = CONN_NONE;
  if (fromF == toF && fromR == toR) return false;

  if (fromF == toF && fromR >= 1 && fromR <= ROOMS_PER_FLOOR &&
      toR >= 1 && toR <= ROOMS_PER_FLOOR && std::abs(fromR - toR) == 1) {
    int leftRoomIdx = (fromR < toR ? fromR : toR) - 1;
    outConnType = horizontalConn[fromF - 1][leftRoomIdx];
    return outConnType != CONN_NONE && outConnType != CONN_SOLID_WALL;
  }

  if (fromR == 1 && toR == 1 &&
      ((fromF == 1 && toF == 2) || (fromF == 2 && toF == 1) ||
       (fromF == 2 && toF == 3) || (fromF == 3 && toF == 2) ||
       (fromF == 4 && toF == 5) || (fromF == 5 && toF == 4))) {
    outConnType = 'S';
    return true;
  }

  if ((fromF == 1 || fromF == 2) && (toF == 1 || toF == 2) &&
      (fromR == 2 || fromR == 3) && (toR == 2 || toR == 3)) {
    outConnType = '1';
    return true;
  }

  if (fromR == 4 && toR == 4 &&
      (fromF == 2 || fromF == 4 || fromF == 5) &&
      (toF == 2 || toF == 4 || toF == 5)) {
    outConnType = '2';
    return true;
  }

  return false;
}

bool canTraverse(int targetFloor, int targetRoom, char connType,
                 const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,
                 int emergencyTripsLeft, bool lift2Operational) {
  (void)targetFloor;
  (void)targetRoom;
  if (connType == CONN_LOCKED_DOOR) return unlockFloor3Door(inventory, inventoryCount);
  if (connType == '1') return true;
  if (connType == '2') return lift2Operational || emergencyTripsLeft > 0;
  return connType == CONN_DOOR || connType == CONN_WINDOW || connType == 'S';
}

void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime) {
  energy = clampInt(energy + deltaEnergy, 0, MAX_ENERGY);
  time = clampInt(time + deltaTime, 0, MAX_ENERGY);
}

void applyScoreDelta(int& score, int deltaScore) {
  score += deltaScore;
}
