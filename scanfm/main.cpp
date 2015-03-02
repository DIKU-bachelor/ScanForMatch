#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm>
#include <stdio.h>
using namespace std;

#define fsize 250000000
#define MAX_PATS 1000
#define EXACT_PUNIT 1

class PatUnit {
  public:
    PatUnit(int);
    int type;
    union {
      struct exact_info {
        int len;
        const char* code;
      } exact;
    } info;
};

PatUnit::PatUnit(int typ) {
  type = typ;
}

void parse_patterns(string text, list<PatUnit> patlist) {
  int pos = text.find(" ");

  /* If there is only one pattern unit */
  if (pos < 0 || text[pos + 1] == '\n' || text[pos + 1] == '\t' || text[pos + 1] == '\r') {
    string trim_t;
    for (int i = 0; i < text.length(); i++) {
      if (text[i] != '\n' && text[i] != '\t' && text[i] != '\r' && text[i] != ' ') {
        trim_t += text[i];
      }
    }
    return;
  }
  else {
    cout << "Multiple patterns\n";
    return;
  }
}

int main(int argc, char* argv[]) {
  ifstream fpp (argv[1]);
  char* data = new char[fsize];
  char* sdata = data;
  while (fpp.get(*data)) {
    data++;
  }
  cout << sdata;
  string text = sdata;
  list<PatUnit> patlist;
  parse_patterns(text, patlist);
}
