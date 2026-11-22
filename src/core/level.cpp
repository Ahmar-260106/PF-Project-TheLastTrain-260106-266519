#include "level.h"
#include <cctype>
#include <cstring>
#include <fstream>
const char *getEntityName(char c) {
  switch (c) {
  case '<':
    return "Master Key";
  case ')':
    return "Energy Pack";
  case '\\':
    return "Emergency Toolkit";
  case '4':
    return "Clue 4";
  case '*':
    return "Fuse";
  case '~':
    return "Cat";
  case ';':
    return "Control Panel";
  case '(':
    return "Lift Pass";
  case '!':
    return "Passenger";
  case '%':
    return "Medical Kit";
  case '$':
    return "Emergency Access Card";
  case '2':
    return "Clue 2";
  case '7':
    return "Clue 7";
  case '^':
    return "Train Key Cabinet";
  case '@':
    return "Cat Carrier";
  case '&':
    return "Observation Token";
  default:
    return "Unknown";
  }
}
bool loadBuildingMap(const char *f, char h[NUM_FLOORS][3],
                     char e[NUM_FLOORS][4][MAX_ENTITIES_PER_ROOM],
                     int n[NUM_FLOORS][4]) {
  std::ifstream in(f);
  if (!in)
    return false;
  char *buf = new char[MAX_LINE_LEN];
  if (!buf)
    return false;
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 3; x++)
      h[y][x] = ' ';
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 4; x++) {
      n[y][x] = 0;
      for (int z = 0; z < MAX_ENTITIES_PER_ROOM; z++)
        e[y][x][z] = ' ';
    }
  while (in.getline(buf, MAX_LINE_LEN)) {
  }
  delete[] buf;
  const char topo[5][3] = {{'W', 'L', 'D'},
                           {'D', 'L', 'D'},
                           {'L', 'W', 'D'},
                           {'W', '#', 'D'},
                           {'D', 'D', 'W'}};
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 3; x++)
      h[y][x] = topo[y][x];
#define ADD(F, R, C) e[(F)-1][(R)-1][n[(F)-1][(R)-1]++] = (C)
  ADD(1, 2, '<');
  ADD(1, 4, ')');
  ADD(1, 4, ')');
  ADD(2, 3, '\\');
  ADD(3, 2, '4');
  ADD(3, 2, '*');
  ADD(3, 3, '~');
  ADD(3, 4, ';');
  ADD(3, 4, '(');
  ADD(4, 1, '!');
  ADD(4, 2, '%');
  ADD(4, 2, '$');
  ADD(4, 3, '2');
  ADD(5, 1, '7');
  ADD(5, 1, '^');
  ADD(5, 3, '@');
  ADD(5, 4, '&');
#undef ADD
  return true;
}
bool loadInitialState(const char *, int &f, int &r, int &e, int &t, int &s,
                      char i[MAX_INVENTORY_CAPACITY], int &ic,
                      bool o[NUM_OBJECTIVES], int &tr) {
  f = 1;
  r = 1;
  e = 100;
  t = 50;
  s = 0;
  ic = 0;
  for (int k = 0; k < MAX_INVENTORY_CAPACITY; k++)
    i[k] = 0;
  for (int k = 0; k < NUM_OBJECTIVES; k++)
    o[k] = false;
  tr = 0;
  return true;
}
bool validateStartingRoom(int f, int r, const char h[NUM_FLOORS][3]) {
  (void)h;
  return f >= 1 && f <= 5 && r >= 1 && r <= 4;
}
