#include "switches.h"
#include "physics.h"
#include <cstring>
bool canUseLift1(int a, int b) {
  return (a == 1 && b == 2) || (a == 2 && b == 1);
}
bool canUseLift2(int a, int b, bool active, int trips) {
  return ((a == 2 || a == 4 || a == 5) && (b == 2 || b == 4 || b == 5) &&
          a != b) &&
         (active || trips > 0);
}
bool unlockFloor3Door(const char i[MAX_INVENTORY_CAPACITY], int n) {
  return hasItem(i, n, ENT_MASTER_KEY);
}
bool installFuseControlPanel(char i[MAX_INVENTORY_CAPACITY], int &n,
                             bool o[NUM_OBJECTIVES], int &s) {
  if (!removeItem(i, n, ENT_FUSE))
    return false;
  o[OBJ_FUSE_INSTALLED] = true;
  s += SCORE_INSTALL_FUSE;
  if (o[OBJ_LIFT_PASS])
    o[OBJ_LIFT2_ACTIVE] = true;
  return true;
}
bool verifyCabinetCode(const char *c, const bool o[NUM_OBJECTIVES]) {
  return c && std::strcmp(c, "427") == 0 && o[OBJ_CLUE_4] && o[OBJ_CLUE_2] &&
         o[OBJ_CLUE_7];
}
