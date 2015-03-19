#include "Punit.h" 
#include <fstream>
#include <iostream>
#include "stdio.h"
#include "ctype.h"
#include <list>
#include <cstdio>
#include <cerrno>
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
list<Punit*> parse(string text, char* end_of_data, int data_len) {
  list<string> split_text = split_str(text, ' ');
  list<Punit*> pat_list;
  char x[] = {'A','C','G','T','U','M','R','W','S','Y','K','B','D','H','V','N'};
  list<char> known_chars (x, x + 16);
  for (list<string>::iterator it = split_text.begin(); it != split_text.end(); it++) {
    int valid_punit = 1;
    /* Checks if the Punit is an exact */
    int count = 0;
    for (int i = 0; i < (*it).length(); i++) {
      if (find(known_chars.begin(), known_chars.end(), toupper((*it)[i])) == known_chars.end()) {
        valid_punit = 0;
        break;
      }
      count++;
    }
    // Range unit
    if (valid_punit == 0) {
      string min = (*it).substr(0, (*it).find('.'));
      string max = (*it).substr((*it).find("..") + 2);
      Range* ra = new Range(end_of_data, atoi(min.c_str()), NULL, atoi(max.c_str()) - atoi(min.c_str()));
      pat_list.push_back(ra); 
    }

    /* Converting to _BIT chars */
    if (count == (*it).length()) {
      char* conv_code = new char[1000];
      char* temp = new char[(*it).length() + 1];
      strcpy(temp, (*it).c_str());
      for(int i = 0; i < (*it).length(); i++){
        conv_code[i] = punit_to_code[temp[i]];
      }
      Exact* ex = new Exact(end_of_data, data_len, (int) (*it).length(), conv_code, 0, 0, 0, 0);
      pat_list.push_back(ex);
    }
  }
  return pat_list; 
}

/* Match struct that contains a pointer to the start of the match and the length */
struct match {
  string start;
  int pos;
};

/* Looks through the data string to find pattern specified in list pat_list */
void pattern_match(list<Punit*> pat_list, char* data, char* real_data, char* end_of_data) {
  list<Punit*>::iterator it = pat_list.begin();
  ret_t* retu = new ret();
  retu->startp = data;
  int dist;
  int data_len = end_of_data - data;
  retu->len = data_len;
  retu->match_len = 0;
  char* match_start;
  char* start_of_data = data;
  int l = 0;
  while (true) {
    if (it == pat_list.begin()) {
      retu->len = data_len;
    }
    retu = (*it)->search(retu);
    // If the punit matched
    if (retu->startp) {
//      cout << "punit match\n";
      // If the whole pattern matched
      if (++it == pat_list.end()) {
//        cout << "whole pattern match\n\n";
        dist = (retu->startp - retu->match_len) - start_of_data;
        printf("%i  %.*s\n", dist, retu->match_len, real_data + dist);
        it = pat_list.begin();
        data = retu->startp;
        retu->len = data_len;
        retu->match_len = 0;
        continue;
      }
      data = retu->startp;
      continue;
    // If the punit didn't match
    } else {
//      cout << "punit NOT match\n";
      // If whole pattern didn't match
      if (it == pat_list.begin()) {
//        cout << "whole pattern NOT match\n\n";
        // If there is no more data
        return;
      }
      if (--it == pat_list.begin()) {
      }
    }
  }
}

/*
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
    printf("data: %s, datap: %p\n", mData, &mData);
    Exact exact = Exact(end_of_data, len, mPattern, 
                        0, 0, 0, 0);
    ret_t* hit;
    int d = 0; 
      exact.mlen = 0;
      ret_t* r = (ret_t*)malloc(sizeof(ret_t));
      r->startp = (mData + d);
      r->len = 30;
      hit = exact.search(r);
      if(hit->startp != NULL) {
        printf("first data letter:%c number: %i \n", *hit->startp, d);
      } else {
       printf("failed\n");
      }
    return 0;
}
 
// unit test that range returns the right pointer after jump 

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
    ret_t* next = (ret_t*)malloc(sizeof(ret_t));
    next->startp = mData;
    next->len = 0;
    next = range.search(next);
    printf("range result startp = %p, len = %i \n", next->startp, next->len);
    return 0;
} */
/*
int test_complementary()
{
    int len = 3;
    char pattern[] = "GTT";
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
    Complementary comple = Complementary(end_of_data, len, mPattern, 
                        1, 0, 0, 1);
    char* hit;
    int d; 
    for(d = 0; d < 30; d++){
      hit = comple.search(mData + d);
      if(hit != NULL) {
        printf("first data letter:%c number: %i \n", *hit, d);
      } else {
       printf("failed\n");
      }
    }
    return 0;
} */


int main(int argc, char* argv[]) {
  build_conversion_tables(); 
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
  pats[--index] = '\0';

  // Read the datafile 
  string real_data;
  char* data = new char[fsize];
  char* rdata = new char[fsize];
  char* end_of_data = data;
  FILE* fp = fopen(argv[++arg], "rb");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    real_data.resize(ftell(fp));
    rewind(fp);
    fread(&real_data[0], 1, real_data.size(), fp);
    fclose(fp);
  } else {
    cout << "ERROR: No such file: \n" << argv[arg] << "\n";
    return -1;
  }
  real_data.erase(remove(real_data.begin(), real_data.end(), '\n'), real_data.end());
  strcpy(rdata, real_data.c_str());
  int i;
  for (i = 0; i < real_data.size(); i++) {
    data[i] = punit_to_code[tolower(real_data[i])];
    end_of_data++;
  }
  data[++i] = '\0';
  // Parse Punits
  list<Punit*> pat_list = parse(pats, end_of_data, end_of_data - data);

//  cout << "DATA INPUT: " << real_data << "\n";
  // Pattern matching
  cout << "FØR PATTERN MATCH\n";
  pattern_match(pat_list, data, rdata, end_of_data);
/*
  cout << "MATHCES:\n\n";
  for (list<match>::iterator itt = matches.begin(); itt != matches.end(); itt++) {
    cout << (*itt).start << " " << (*itt).pos << "\n";
  } */ 
  return 0;
}
