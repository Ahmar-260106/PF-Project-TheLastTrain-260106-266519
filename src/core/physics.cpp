#include "physics.h"
#include "switches.h"
#include "util.h"
#include <cstdlib>
bool hasItem(const char i[MAX_INVENTORY_CAPACITY], int n, char c) {
  for (int k = 0; k < n; k++)
    if (i[k] == c)
      return true;
  return false;
}
bool addItem(char i[MAX_INVENTORY_CAPACITY], int &n, char c) {
  if (hasItem(i, n, c))
    return true;
  if (n >= MAX_INVENTORY_CAPACITY)
    return false;
  i[n++] = c;
  return true;
}
bool removeItem(char i[MAX_INVENTORY_CAPACITY], int &n, char c) {
  for (int k = 0; k < n; k++)
    if (i[k] == c) {
      for (int j = k; j < n - 1; j++)
        i[j] = i[j + 1];
      i[--n] = 0;
      return true;
    }
  return false;
}
bool isDirectlyConnected(int ff, int fr, int tf, int tr, const char h[5][3],
                         char &c) {
  c = ' ';
  if (ff == tf && fr >= 1 && fr <= 4 && tr >= 1 && tr <= 4 &&
      std::abs(fr - tr) == 1) {
    c = h[ff - 1][(fr < tr ? fr : tr) - 1];
    return c != ' ' && c != '#';
  }
  if (fr == 1 && tr == 1 &&
      ((ff == 1 && tf == 2) || (ff == 2 && tf == 1) || (ff == 2 && tf == 3) ||
       (ff == 3 && tf == 2) || (ff == 4 && tf == 5) || (ff == 5 && tf == 4))) {
    c = 'S';
    return true;
  }
  if (((ff == 1 && tf == 2) || (ff == 2 && tf == 1)) && (fr == 2 || fr == 3) &&
      (tr == 2 || tr == 3)) {
    c = '1';
    return true;
  }
  if (fr == 4 && tr == 4 &&
      ((ff == 2 && tf == 4) || (ff == 4 && tf == 2) || (ff == 2 && tf == 5) ||
       (ff == 5 && tf == 2) || (ff == 4 && tf == 5) || (ff == 5 && tf == 4))) {
    c = '2';
    return true;
  }
  return false;
}
bool canTraverse(int f, int r, char c, const char i[MAX_INVENTORY_CAPACITY],
                 int n, int trips, bool active) {
  (void)f;
  (void)r;
  if (c == 'L')
    return unlockFloor3Door(i, n);
  if (c == '1')
    return true;
  if (c == '2')
    return active || trips > 0;
  return c == 'D' || c == 'W' || c == 'S';
}
void applyResourceDelta(int &e, int &t, int de, int dt) {
  e = clampInt(e + de, 0, 100);
  t = clampInt(t + dt, 0, 100);
}
void applyScoreDelta(int &s, int d) { s += d; }
