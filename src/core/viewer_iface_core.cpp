#include "level.h"
#include "viewer_iface.h"
#include <iostream>
void renderDashboard(int f, int r, int e, int t, int s,
                     const char i[MAX_INVENTORY_CAPACITY], int n, int trips) {
  std::cout << "F" << f << "-R" << r << " | Energy " << e << " | Time " << t
            << " | Score " << s << " | Inventory ";
  for (int k = 0; k < n; k++)
    std::cout << i[k] << ' ';
  std::cout << "| Lift 2 trips " << trips << "\n";
}
void renderRoomDetails(int f, int r, const char e[5][4][MAX_ENTITIES_PER_ROOM],
                       int n) {
  std::cout << "Contents F" << f << "-R" << r << ": ";
  for (int k = 0; k < n; k++)
    std::cout << e[f - 1][r - 1][k] << " (" << getEntityName(e[f - 1][r - 1][k])
              << ") ";
  std::cout << "\n";
}
void renderAsciiBuilding(int f, int r) {
  std::cout << "BUILDING (D door, W window, L locked, # wall, S stairs, L1/L2 "
               "lifts)\n";
  for (int y = 5; y >= 1; y--) {
    for (int x = 1; x <= 4; x++) {
      std::cout << "[" << (f == y && r == x ? "P" : " ") << " F" << y << "-R"
                << x << "]";
      if (x < 4)
        std::cout << "-";
    }
    std::cout << "\n";
  }
}
void renderObjectivesChecklist(const bool o[NUM_OBJECTIVES]) {
  const char *n[] = {"Master Key",
                     "Clue 4",
                     "Clue 2",
                     "Clue 7",
                     "Cabinet Unlocked",
                     "Train Key",
                     "Fuse Collected",
                     "Fuse Installed",
                     "Lift Pass",
                     "Lift 2 Active",
                     "Passenger Rescued",
                     "Cat Carrier",
                     "Cat Secured",
                     "Train Prepared",
                     "Mission Won",
                     "Mission Failed"};
  std::cout << "Objectives: ";
  for (int k = 0; k < NUM_OBJECTIVES; k++)
    std::cout << "[" << (o[k] ? 'X' : ' ') << "]" << n[k] << " ";
  std::cout << "\n";
}
