#include "util.h"
#include <cstring>
#include <iostream>
int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
void trimWhitespace(char *s) {
  if (!s)
    return;
  int n = (int)std::strlen(s);
  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\n' || s[n - 1] == '\r' ||
                   s[n - 1] == '\t'))
    s[--n] = '\0';
  int i = 0;
  while (s[i] == ' ' || s[i] == '\t')
    ++i;
  if (i)
    std::memmove(s, s + i, std::strlen(s + i) + 1);
}
bool stringEquals(const char *a, const char *b) {
  return a && b && std::strcmp(a, b) == 0;
}
void printHeader(const char *t) {
  std::cout << "\n=== " << (t ? t : "") << " ===\n";
}
void clearScreen() {}
