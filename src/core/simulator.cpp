#include "simulator.h"
#include "level.h"
#include "logger.h"
#include "physics.h"
#include "switches.h"
#include "util.h"
#include "viewer_iface.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
static void eraseEntity(char a[5][4][MAX_ENTITIES_PER_ROOM], int f, int r,
                        int &n, int idx) {
  for (int j = idx; j < n - 1; j++)
    a[f - 1][r - 1][j] = a[f - 1][r - 1][j + 1];
  a[f - 1][r - 1][--n] = ' ';
}
bool handleMoveCommand(int tf, int tr, int &cf, int &cr, int &e, int &t,
                       const char h[5][3], const char i[MAX_INVENTORY_CAPACITY],
                       int ic, int &trips, bool active) {
  char c;
  if (tf < 1 || tf > 5 || tr < 1 || tr > 4 ||
      !isDirectlyConnected(cf, cr, tf, tr, h, c) ||
      !canTraverse(tf, tr, c, i, ic, trips, active))
    return false;
  int de = -2, dt = -1;
  if (c == 'W') {
    de = -3;
    dt = -2;
  }
  if (c == '1' || c == '2') {
    de = -4;
    dt = -3;
  }
  applyResourceDelta(e, t, de, dt);
  if (c == '2' && !active)
    --trips;
  cf = tf;
  cr = tr;
  return true;
}
bool handleInspectCommand(int f, int r, int &e, int &t,
                          const char a[5][4][MAX_ENTITIES_PER_ROOM], int n,
                          bool o[NUM_OBJECTIVES], int &s) {
  applyResourceDelta(e, t, -1, -1);
  for (int k = 0; k < n; k++) {
    char c = a[f - 1][r - 1][k];
    int z = c == '4'   ? OBJ_CLUE_4
            : c == '2' ? OBJ_CLUE_2
            : c == '7' ? OBJ_CLUE_7
                       : -1;
    if (z >= 0 && !o[z]) {
      o[z] = true;
      s += 10;
    }
  }
  return true;
}
bool handleCollectCommand(char c, int f, int r, int &e, int &t, int &s,
                          char a[5][4][MAX_ENTITIES_PER_ROOM], int &n,
                          char i[MAX_INVENTORY_CAPACITY], int &ic,
                          bool o[NUM_OBJECTIVES]) {
  int q = -1;
  for (int k = 0; k < n; k++)
    if (a[f - 1][r - 1][k] == c) {
      q = k;
      break;
    }
  if (q < 0)
    return false;
  if (c == '!' || c == '~' || c == ';' || c == '^' || c == '4' || c == '2' ||
      c == '7') {
    std::cout << "That fixture cannot be collected.\n";
    return false;
  }
  if (c == ')' || c == '\\' || c == '&') {
    applyResourceDelta(e, t, -1, -1);
    if (c == ')')
      e = clampInt(e + 25, 0, 100);
    if (c == '\\')
      e = clampInt(e + 15, 0, 100);
    if (c == '&')
      t = clampInt(t + 15, 0, 100);
  } else {
    if (!addItem(i, ic, c))
      return false;
    applyResourceDelta(e, t, -1, -1);
    if (c == '<') {
      o[OBJ_MASTER_KEY] = true;
      s += 10;
    }
    if (c == '*')
      o[OBJ_FUSE_COLLECTED] = true;
    if (c == '(') {
      o[OBJ_LIFT_PASS] = true;
      s += 20;
      if (o[OBJ_FUSE_INSTALLED])
        o[OBJ_LIFT2_ACTIVE] = true;
    }
    if (c == '$') {
    }
    if (c == '@')
      o[OBJ_CAT_CARRIER] = true;
  }
  eraseEntity(a, f, r, n, q);
  return true;
}
bool handleUseCommand(char c, int f, int r, int &e, int &t, int &s,
                      char i[MAX_INVENTORY_CAPACITY], int &ic,
                      bool o[NUM_OBJECTIVES]) {
  if (!hasItem(i, ic, c))
    return false;
  if (c == '*' && f == 3 && r == 4) {
    if (!installFuseControlPanel(i, ic, o, s))
      return false;
    if (o[OBJ_LIFT_PASS])
      o[OBJ_LIFT2_ACTIVE] = true;
  } else if (c == '%' && f == 4 && r == 1) {
    removeItem(i, ic, c);
    o[OBJ_PASSENGER_RESCUED] = true;
    s += 100;
  } else if (c == '@' && f == 3 && r == 3) {
    removeItem(i, ic, c);
    o[OBJ_CAT_SECURED] = true;
    s += 100;
  } else if (c == '^' && f == 5 && r == 2 && o[OBJ_TRAIN_KEY] &&
             o[OBJ_PASSENGER_RESCUED] && o[OBJ_CAT_SECURED]) {
    applyResourceDelta(e, t, -1, -1);
    s += 200;
    o[OBJ_TRAIN_PREPARED] = o[OBJ_MISSION_WON] = true;
    return true;
  } else
    return false;
  applyResourceDelta(e, t, -1, -1);
  return true;
}
bool handleUnlockCabinetCommand(const char *c, int f, int r,
                                bool o[NUM_OBJECTIVES], int &s,
                                char a[5][4][MAX_ENTITIES_PER_ROOM], int &n) {
  (void)a;
  (void)n;
  if (f != 5 || r != 1 || o[OBJ_CABINET_UNLOCKED] || !verifyCabinetCode(c, o))
    return false;
  o[OBJ_CABINET_UNLOCKED] = o[OBJ_TRAIN_KEY] = true;
  s += 40;
  return true;
}
bool checkEndConditions(int e, int t, const bool o[NUM_OBJECTIVES]) {
  return e <= 0 || t <= 0 || o[OBJ_MISSION_WON] || o[OBJ_MISSION_FAILED];
}
void runSimulation(int &f, int &r, int &e, int &t, int &s,
                   char i[MAX_INVENTORY_CAPACITY], int &ic,
                   bool o[NUM_OBJECTIVES], int &trips, char h[5][3],
                   char a[5][4][MAX_ENTITIES_PER_ROOM], int n[5][4]) {
  std::string line;
  std::cout << "Starting floor (1-5): ";
  if (std::getline(std::cin, line)) {
    std::istringstream z(line);
    int sf = 0, sr = 0;
    if (z >> sf) {
      if (!(z >> sr)) {
        std::cout << "Starting room (1-4): ";
        if (std::getline(std::cin, line)) {
          std::istringstream roomInput(line);
          roomInput >> sr;
        }
      }
      if (validateStartingRoom(sf, sr, h)) {
        f = sf;
        r = sr;
      }
    }
  }
  bool done = false;
  while (!done) {
    renderDashboard(f, r, e, t, s, i, ic, trips);
    std::cout << "> ";
    if (!std::getline(std::cin, line))
      line = "quit";
    std::istringstream ss(line);
    std::string cmd, arg;
    ss >> cmd;
    if (cmd == "m" || cmd == "move") {
      int x, y;
      if (ss >> x >> y) {
        if (!handleMoveCommand(x, y, f, r, e, t, h, i, ic, trips,
                               o[OBJ_LIFT2_ACTIVE]))
          std::cout << "Move rejected.\n";
      }
    } else if (cmd == "i" || cmd == "inspect") {
      handleInspectCommand(f, r, e, t, a, n[f - 1][r - 1], o, s);
      renderRoomDetails(f, r, a, n[f - 1][r - 1]);
    } else if (cmd == "c" || cmd == "collect") {
      ss >> arg;
      if (arg.size() == 1) {
        bool card = (arg[0] == '$');
        if (!handleCollectCommand(arg[0], f, r, e, t, s, a, n[f - 1][r - 1], i,
                                  ic, o))
          std::cout << "Item unavailable or cannot be collected.\n";
        else if (card)
          trips = 3;
      }
    } else if (cmd == "u" || cmd == "use") {
      ss >> arg;
      if (arg.size() == 1 && !handleUseCommand(arg[0], f, r, e, t, s, i, ic, o))
        std::cout << "Cannot use that item here.\n";
    } else if (cmd == "unlock" || cmd == "code") {
      ss >> arg;
      if (handleUnlockCabinetCommand(arg.c_str(), f, r, o, s, a,
                                     n[f - 1][r - 1])) {
        addItem(i, ic, '^');
        applyResourceDelta(e, t, -1, -1);
        std::cout << "Cabinet unlocked.\n";
      } else
        std::cout << "Code rejected or clues missing.\n";
    } else if (cmd == "save")
      saveGameStateToFile("out/savegame.txt", f, r, e, t, s, i, ic, o, trips, a,
                          n);
    else if (cmd == "load")
      loadGameStateFromFile("out/savegame.txt", f, r, e, t, s, i, ic, o, trips,
                            a, n);
    else if (cmd == "map")
      renderAsciiBuilding(f, r);
    else if (cmd == "status")
      renderObjectivesChecklist(o);
    else if (cmd == "help")
      std::cout << "move/m F R, inspect/i, collect/c SYMBOL, use/u SYMBOL, "
                   "unlock/code CODE, save, load, map, status, quit\n";
    else if (cmd == "quit")
      done = true;
    else if (!cmd.empty())
      std::cout << "Unknown command.\n";
    if (!o[OBJ_MISSION_WON] && (e <= 0 || t <= 0)) {
      o[OBJ_MISSION_FAILED] = true;
      done = true;
    }
    if (o[OBJ_MISSION_WON])
      done = true;
  }
  saveGameStateToFile("out/savegame.txt", f, r, e, t, s, i, ic, o, trips, a, n);
  writeGameSummaryReport("out/game_summary.txt", s, e, t, o,
                         o[OBJ_MISSION_WON]);
}
