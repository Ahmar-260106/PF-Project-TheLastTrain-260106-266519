#include "logger.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <string>
static const char *names[NUM_OBJECTIVES] = {"Master Key",
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
bool saveGameStateToFile(const char *f, int pf, int pr, int en, int ti, int sc,
                         const char inv[MAX_INVENTORY_CAPACITY], int ic,
                         const bool o[NUM_OBJECTIVES], int trips,
                         const char ent[5][4][MAX_ENTITIES_PER_ROOM],
                         const int cnt[5][4]) {
  std::ofstream x(f);
  if (!x)
    return false;
  x << "floor=" << pf << "\nroom=" << pr << "\nenergy=" << en << "\ntime=" << ti
    << "\nscore=" << sc << "\ntrips=" << trips << "\ninventory=";
  for (int k = 0; k < ic; k++)
    x << inv[k];
  x << "\n";
  for (int k = 0; k < NUM_OBJECTIVES; k++)
    x << "objective" << k << "=" << o[k] << "\n";
  for (int a = 0; a < 5; a++)
    for (int b = 0; b < 4; b++) {
      x << "room" << a << "_" << b << "=" << cnt[a][b] << ":";
      for (int c = 0; c < cnt[a][b]; c++)
        x << ent[a][b][c];
      x << "\n";
    }
  return (bool)x;
}
bool loadGameStateFromFile(const char *f, int &pf, int &pr, int &en, int &ti,
                           int &sc, char inv[MAX_INVENTORY_CAPACITY], int &ic,
                           bool o[NUM_OBJECTIVES], int &trips,
                           char ent[5][4][MAX_ENTITIES_PER_ROOM],
                           int cnt[5][4]) {
  std::ifstream x(f);
  if (!x)
    return false;
  std::string line;
  while (std::getline(x, line)) {
    size_t p = line.find('=');
    if (p == std::string::npos)
      continue;
    std::string k = line.substr(0, p), v = line.substr(p + 1);
    int z = std::atoi(v.c_str());
    if (k == "floor")
      pf = z;
    else if (k == "room")
      pr = z;
    else if (k == "energy")
      en = z;
    else if (k == "time")
      ti = z;
    else if (k == "score")
      sc = z;
    else if (k == "trips")
      trips = z;
    else if (k == "inventory") {
      ic = 0;
      for (char c : v)
        if (ic < MAX_INVENTORY_CAPACITY)
          inv[ic++] = c;
    } else if (k.rfind("objective", 0) == 0) {
      int n = std::atoi(k.c_str() + 9);
      if (n >= 0 && n < NUM_OBJECTIVES)
        o[n] = z != 0;
    } else if (k.rfind("room", 0) == 0) {
      int a = -1, b = -1;
      if (std::sscanf(k.c_str(), "room%d_%d", &a, &b) == 2 && a >= 0 && a < 5 &&
          b >= 0 && b < 4) {
        size_t q = v.find(':');
        cnt[a][b] = std::atoi(v.substr(0, q).c_str());
        for (int j = 0; j < cnt[a][b] && j < MAX_ENTITIES_PER_ROOM; j++)
          ent[a][b][j] = q != std::string::npos && q + 1 + (size_t)j < v.size()
                             ? v[q + 1 + j]
                             : ' ';
      }
    }
  }
  return true;
}
void writeGameSummaryReport(const char *f, int s, int e, int t,
                            const bool o[NUM_OBJECTIVES], bool win) {
  std::ofstream x(f);
  if (!x)
    return;
  x << "THE LAST TRAIN - " << (win ? "VICTORY" : "DEFEAT / QUIT")
    << "\nScore: " << s << " / 520\nEnergy: " << e << "\nTime: " << t
    << "\nObjectives:\n";
  for (int k = 0; k < NUM_OBJECTIVES; k++)
    x << "[" << (o[k] ? "X" : " ") << "] " << names[k] << "\n";
}
