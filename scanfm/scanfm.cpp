#include "Punit.h"
#include <fstream>
#include <iostream>
#include "stdio.h"
#include "ctype.h"
#include <list>
#include <string>
#include <algorithm>
using namespace std;

#define fsize 250000000

/* Splits text into substrings by delimiter del and returns a list of these */
list<string> split_str(string text, const char del) {
  list<string> pat_list;
  if (text.length() < 1) {
    return pat_list;
  }
  string pat;
  int pos = text.find(del);
  while (pos != -1) {
    pat = text.substr(0, pos);
    pat_list.push_back(pat);
    text.erase(0, pos + 1);
    pos = text.find(del);
  }
  /* Removing unwanted characters from last pattern unit */
  for (int i = 0; i < text.length(); i++) {
    if (text[i] == '\n' || text[i] == '\r' || text[i] == '\t' || text[i] == '\0' 
      || text[i] == ' ') {
      text.erase(i, 1);
    }
  }
  pat_list.push_back(text);
  return pat_list;
}

/* Parses text to find pattern units and returns a list of these */
list<Punit> parse(string text) {
  list<string> split_text = split_str(text, ' ');
  list<Punit> pat_list;
  char x[] = {'A','C','G','T','U','M','R','W','S','Y','K','B','D','H','V','N'};
  list<char> known_chars (x, x + 16);
  for (list<string>::iterator it = split_text.begin(); it != split_text.end(); it++) {
    int count = 0;
    for (int i = 0; i < (*it).length(); i++) {
      if (find(known_chars.begin(), known_chars.end(), toupper((*it)[i])) == known_chars.end()) {
        break;
      }
      count++;
    }
    if (count == (*it).length()) {
      Exact ex ((int) (*it).length(), (*it).c_str(), 0, 0, 0, 0);
      pat_list.push_back(ex);
    }
  }
  return pat_list; 
}


int main(int argc, char* argv[]) {
  ifstream fpp (argv[1]);
  char* data = new char[fsize];
  char* sdata = data;
  while (fpp.get(*data)) {
    data++;
  }
  cout << "Pattern input: "<< sdata << '\n';
  string text = sdata;
  list<Punit> pat_list = parse(text);
  for (list<Punit>::iterator it = pat_list.begin(); it != pat_list.end(); it++) {
    cout << (*it).code << '\n';
  }
}
