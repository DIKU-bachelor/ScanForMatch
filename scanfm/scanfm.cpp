#include "Punit.h"
#include <fstream>
#include <iostream>
#include "stdio.h"
#include "ctype.h"
#include <list>
#include <cstring>
#include <typeinfo>
#include <string>
#include <algorithm>
using namespace std;

#define fsize 250000000
#define patsize 1000

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
list<Punit*> parse(string text, char* end_of_data) {
  list<string> split_text = split_str(text, ' ');
  list<Punit*> pat_list;
  char x[] = {'A','C','G','T','U','M','R','W','S','Y','K','B','D','H','V','N'};
  list<char> known_chars (x, x + 16);
  for (list<string>::iterator it = split_text.begin(); it != split_text.end(); it++) {
    /* Checks if the Punit is an exact */
    int count = 0;
    for (int i = 0; i < (*it).length(); i++) {
      if (find(known_chars.begin(), known_chars.end(), toupper((*it)[i])) == known_chars.end()) {
        break;
      }
      count++;
    }
    /* Converting to _BIT chars */
    if (count == (*it).length()) {
      char* conv_code = new char[1000];
      char* temp = new char[1000];
      strcpy(temp, (*it).c_str());
      for(int i = 0; i < (*it).length(); i++){
        conv_code[i] = punit_to_code[temp[i]];
      }
      Exact* ex = new Exact(end_of_data, (int) (*it).length(), conv_code, 0, 0, 0, 0);
      pat_list.push_back(ex);
    }
  }
  return pat_list; 
}

/* Match struct that contains a pointer to the start of the match and the length */
struct match {
  char* start;
  int dist;
};

/* Looks through the data string to find pattern specified in list pat_list */
list<match> pattern_match(list<Punit*> pat_list, char* data) {
  list<Punit*>::iterator it = pat_list.begin();
  list<match> matches;
  char* nxt_start = data;
  char* match_start;
  while (true) {
    match_start = nxt_start;
    cout << "loop start\n";
    nxt_start = (*it)->search(nxt_start);
    cout << "after search\n";
    // If the search was succesfull we search for the next punit 
    if (nxt_start) {
      cout << "punit match succes\n";
      // If we matched the whole pattern 
      if (it == pat_list.end()) {
        cout << "whole pattern matched\n";
        match m;
        m.start = match_start;
        m.dist = nxt_start - match_start;
        matches.push_back(m);

        // If there is no more data
        if (strcmp(nxt_start, "\0") == 0) {
          cout <<"End of data, returning list of matches\n";
          return matches;
        }
        cout << "Last punit matched, continuing search...\n";
        it = pat_list.begin();
        continue;
      }
      it++;
      continue;
    }

    // If search was unsuccesfull we iterate back to the previous punit to try again
    else {
      cout << "punit NOT match\n";
      // If we need to backtrack but we're at the first punit 
      if (it == pat_list.begin()) {
        cout << "No match for punit\n";
        if (strcmp(++data, "\0") == 0) {
          cout << "No more data, returning list of matches\n";
          return matches;
        }
        cout << "UOUOUUOU\n";
        nxt_start = data;
        continue;
      }
      it--;
    }
  }
}

int test_exact()
{
    int len = 4;
    char pattern[] = "CAAA";
    char data[] = "AACAAACAACACAAAAAAAAAAAAAAAACT";
    char* mPattern;
    char* mData;
    mData = (char*)malloc(1000*sizeof(char));
    mPattern = (char*)malloc(1000*sizeof(char));
    int i;
    for(i = 0; i < len; i++){
        mPattern[i] = punit_to_code[pattern[i]];
    }
    char* end_of_data;
    for(i = 0; i < 30; i ++) {
        mData[i] = punit_to_code[data[i]];
        end_of_data = &mData[i];
    }
    printf("data: %s\n", mData);
    Exact exact = Exact(end_of_data, len, mPattern, 
                        3, 2, 1, 1);
    char* hit;
    int d; 
    for(d = 0; d < 30; d++){
      exact.mlen = 0;
      hit = exact.search(mData + d);
      if(hit != NULL) {
        printf("first data letter:%c number: %i \n", *hit, d);
      } else {
       printf("failed\n");
      }
    }
    return 0;
}
/* unit test that range returns the right pointer after jump */
int test_range() {
    int len = 6;
    int width = 4;
    char data[] = "AACAAACAACACAAAAAAAAAAAAAAAA";
    char* mData;
    mData = (char*)malloc(1000*sizeof(char));
    int i;
    char* end_of_data;
    for(i = 0; i < 30; i ++) {
        mData[i] = punit_to_code[data[i]];
        end_of_data = &mData[i];
    }
    Range range = Range(end_of_data,  len, NULL, 
                        width);
    char* next;
    for(i = 0; i < 10; i++){
      next = range.search(mData);
    }
    return 0;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    cout << "ERROR: call to scanfm need arguments in format:\nscanfm -p <pattern> -d <datafile>\n";
    return -1;
  }
  int arg = 0;
  /* Skip until -p flag */
  while (strcmp(argv[arg], "-p") != 0) {
    if (arg > argc) {
      cout << "ERROR: missing -p flag\n";
      return -1;
    }
    arg++;
  }
  if (argc == arg + 1) {
    cout << "ERROR: call to scanfm need arguments in format:\nscanfm -p <pattern> -d <datafile>\n";
    return -1;
  }
  arg++;
  char* pats = new char[patsize];
  char* spats = pats;
  int index = 0;

  /* Read all of the pattern */
  while (strcmp(argv[arg], "-d") != 0) {
    int i = 0;
    while (argv[arg][i] != '\0') {
      pats[index++] = argv[arg][i++];
    }
    pats[index++] = ' ';
    if (++arg == argc) {
      cout << "ERROR: missing -d flag" << "\n";
      return -1;
    }
  }
  pats[index++] = '\0';
  // Read the datafile 
  ifstream fp (argv[++arg]);
  if (fp == 0) {
    cout << "ERROR: No such file: \n" << argv[arg] << "\n";
    return -1;
  }
  build_conversion_tables(); 
  char* data = new char[fsize];
  char* sdata = data;
  int i = 0;
  char * end_of_data;
  while (fp.get(data[i])) {
    data[i] = punit_to_code[tolower(data[i])];
    end_of_data = &data[i];
    i++;
  }
  data[i] = '\0';
  // Parse Punits
  list<Punit*> pat_list = parse(pats, end_of_data);

  cout << "data input: "<< sdata << '\n';
  char* x = new char[100];
  char* y = x;
  string a ("hej martin");
  cout << x << "\n";
  for (int i = 0; i < a.length(); i++) {
    *(x++) = 'd';
    cout << i << " " << y << "\n";
  }
  printf("x%sx\n", y);
  return test_exact();
//  list<match> matches = pattern_match(pat_list, sdata);
  return 0;
/*  return test_range(); */
/*  list<Punit> pat_list = parse(text);
  for (list<Punit>::iterator it = pat_list.begin(); it != pat_list.end(); it++) {
    if (Exact ex = dynamic_cast<Exact>((*it)) {
      cout << ex.code << "and length: " << ex.len << "\n";
    }
  }*/
/*  list<char*> mathces = pattern_match(pat_list, sdata);*/
}
