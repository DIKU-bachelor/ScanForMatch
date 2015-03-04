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
      char* conv_code = (char*)malloc(1000*sizeof(char));
      for(int i = 0; i < (*it).length(); i++){
        conv_code[i] = punit_to_code[tolower((*it)[i])];
      }
      Exact ex ((int) (*it).length(), conv_code, 0, 0, 0, 0);
      pat_list.push_back(ex);
    }
  }
  return pat_list; 
}

int test_exact()
{
    int len = 10;
    char pattern[] = "CAAACAACAC";
    char data[] = "AACAAACAACACAAAAAAAAAAAAAAAA";
    char* mPattern;
    char* mData;
    mData = (char*)malloc(1000*sizeof(char));
    mPattern = (char*)malloc(1000*sizeof(char));
    int i;
    for(i = 0; i < len; i++){
        mPattern[i] = punit_to_code[pattern[i]];
    }
    
    for(i = 0; i < 30; i ++) {
        mData[i] = punit_to_code[data[i]];
    }
    Exact exact = Exact(len, mPattern, 
                        0, 0, 0, 0);
    char* hit;
    int d; 
    for(d = 0; d < 5; d++){
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
    for(i = 0; i < 30; i ++) {
        mData[i] = punit_to_code[data[i]];
    }
    Range range = Range(len, NULL, 
                        width);
    char* next;
    for(i = 0; i < 10; i++){
      next = range.search(mData);
    }
}


int main(int argc, char* argv[]) {
  ifstream fpp (argv[1]);
  build_conversion_tables();
  char* data = new char[fsize];
  char* sdata = data;
  while (fpp.get(*data)) {
    data++;
  }
  cout << "Pattern input: "<< sdata << '\n';
  string text = sdata;

  return test_range();
/*  list<Punit> pat_list = parse(text);
  for (list<Punit>::iterator it = pat_list.begin(); it != pat_list.end(); it++) {
    if (Exact ex = dynamic_cast<Exact>((*it)) {
      cout << ex.code << "and length: " << ex.len << "\n";
    }
  }*/
}
