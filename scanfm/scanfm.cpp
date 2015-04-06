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
list<Punit*> parse(string text, char* start_of_data, char* end_of_data, int data_len) {
  // pat_list: contains the parsed punits to be returned
  list<Punit*> pat_list;

  list<var_t*> var_list;
  list<var_t*>::iterator var_it;
  char* var_name = new char[100];

  list<string> split_text = split_str(text, ' ');
  char x[] = {'A','C','G','T','U','M','R','W','S','Y','K','B','D','H','V','N'};
  list<char> known_chars (x, x + 16);

  int comp;
  char* conv_code;
  int save_next = 0;
  Punit* var_p;
  Punit* var_p_nxt;
  var_t* va;

  // Loop to try and parse punits
  for (list<string>::iterator it = split_text.begin(); it != split_text.end(); it++) {
    string pu = (*it);
    int eq = (*it).find('=');
    string varn;

    // Variable type punit, saves variable name in var_list
    if (eq != string::npos) {
      varn = (*it).substr(0, eq);
      pu = pu.substr(eq + 1);
    }
    int dots = pu.find("...");

    // Range type punit, verifies that both sides of ... are valid digits
    if (dots != string::npos) {
      string min_s = pu.substr(0, dots);
      string max_s = pu.substr(dots + 3);
      for (int i = 0; i < min_s.length(); i++) {
        if ((! isdigit(min_s[i])) || (! isdigit(max_s[i]))) {
          cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1 
            << ": Range type punit must be in format <int>...<int>\n";
          pat_list.clear();
          return pat_list;
        }
      }
      int min = atoi(min_s.c_str());
      int max = atoi(max_s.c_str());
      Range* ra = new Range(start_of_data, end_of_data, end_of_data - start_of_data,
        min, NULL, max - min);
      pat_list.push_back(ra);

      // If previous punit was being set as variable
      if (save_next == 1) {
        va->nxt_punit = ra;
        save_next = 0;
      }

      // If this punit is a variable being set
      if (pu.length() != (*it).length()) {
        va = new var();
        va->name = varn;
        va->var_punit = ra;
        var_list.push_back(va);
        save_next = 1;
      }
      continue;
    }
    if (pu.length() != (*it).length()) {
      cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
        << ": Variable can only be assigned Range type punit\n";
      pat_list.clear();
      return pat_list;
    }

    // Exact type punit, checks validity of each letter and converts to 4-BIT chars
    int count = 0;
    for (int i = 0; i < pu.length(); i++) {
      if (find(known_chars.begin(), known_chars.end(), toupper(pu[i])) == known_chars.end()) {
        break;
      }
      count++;
    }
    int until_brac = pu.find('[');
    if (until_brac == string::npos) {
      until_brac = pu.length();
    }
    if (count == until_brac) {
      conv_code = new char[1000];
      char* temp = new char[until_brac + 1];
      strcpy(temp, pu.c_str());
      for(int i = 0; i < until_brac; i++){
        conv_code[i] = punit_to_code[temp[i]];
      }
      conv_code[pu.length()] = '\0';
    }

    // ex_len used to determine length of Exact punit before [ char
    int brac = pu.find('[');
    int ex_len = pu.length();
    if (brac != string::npos) {
      ex_len = brac;
    }

    // Reference type punit, finds potetial ~ and searches var_list to determine 
    // validity of reference
    if (count != ex_len) {
      comp = pu.find('~');
      if (comp != string::npos) {
        pu = pu.substr(1);
      }
      brac = pu.find('[');
      string until_brac = pu;
      if (brac != string::npos) {
        until_brac = pu.substr(0, brac);
      }

      // Searches for valid variable
      for (var_it = var_list.begin(); var_it != var_list.end(); var_it++) {
        if (until_brac.compare((*var_it)->name) == 0) {
          var_p = (*var_it)->var_punit;
          var_p_nxt = (*var_it)->nxt_punit;
          break;
        }
      }
      if (var_it == var_list.end()) {
          cout << "Could not parse punit: " << pu << " not a known variable\n";
        pat_list.clear();
        return pat_list;
      }
    }

    // Finds mismatches, insertions and deletions whether it's an Exact or a Reference punit
    string mis_s;
    string ins_s;
    string del_s;
    int mis = 0;
    int ins = 0;
    int del = 0;
    brac = pu.find('[');
    if (brac != string::npos) {

      // Fills mis_s, ins_s and del_s with the relevant parts of the "[?,?,?]" string
      pu = pu.substr(brac + 1);
      int com = pu.find(',');
      if (com != string::npos) {
        mis_s = pu.substr(0, com);
        pu = pu.substr(com + 1);
      } else {
        cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
          << ": Mismatches, insertions and deletions must be declared in format:\n<punit>[?,?,?]\n";
          pat_list.clear();
          return pat_list;
      }
      com = pu.find(',');
      if (com != string::npos) {
        ins_s = pu.substr(0, com);
        pu = pu.substr(com + 1);
      } else {
        cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
          << "Mismatches, insertions and deletions must be declared in format:\n<punit>[?,?,?]\n";
          pat_list.clear();
          return pat_list;
      }
      brac = pu.find(']');
      if (brac != string::npos) {
        del_s = pu.substr(0, brac);
      } else {
        cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
          << "Mismatches, insertions and deletions must be declared in format:\n<punit>[?,?,?]\n";
          pat_list.clear();
          return pat_list;
      }

      // Check if all characters in mis_s, ins_s and del_s are valid digits
      int invalid = 0;
      for (int i = 0; i < mis_s.length(); i++) {
        if (! isdigit(mis_s[i])) {
          invalid = 1;
        }
      }
      for (int i = 0; i < ins_s.length(); i++) {
        if (! isdigit(ins_s[i])) {
          invalid = 1;
        }
      }
      for (int i = 0; i < del_s.length(); i++) {
        if (! isdigit(del_s[i])) {
          invalid = 1;
        }
      }
      if (invalid) {
        cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
          << ": Mismatches insertions and deletions must be declared in format:\n"
          << "<punit>[<int>,<int>,<int>]\n";
        pat_list.clear();
        return pat_list;
      }
      mis = atoi(mis_s.c_str());
      ins = atoi(ins_s.c_str());
      del = atoi(del_s.c_str());     
    }

    // Checks if it's an Exact or Reference type punit
    if (ex_len == count) {
      Exact* ex = new Exact(start_of_data, end_of_data, data_len, (int) (*it).length(), 
        conv_code, mis, ins, del, 0);
      pat_list.push_back(ex);
      if (save_next == 1) {
        va->nxt_punit = ex;
        save_next = 0;
      }
      continue;
    } 
    if (var_it != var_list.end()) {
      int complem = 0;
      if (comp != string::npos) {
        complem = 1;
      }
      Reference* re = new Reference(start_of_data, end_of_data, data_len, complem, (Range*)
        var_p, var_p_nxt, mis, ins, del, 0);
      pat_list.push_back(re);
      if (! re->next_Punit) {
        re->next_Punit = re;
      }
      if (save_next == 1) {
        va->nxt_punit = re;
        save_next = 0;
      }
      continue;
    } else {
      cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
        << ": Invalid characters in Exact type punit\n";
      pat_list.clear();
      return pat_list;
    }
  }
  return pat_list; 
}


/* Looks through the data string to find pattern specified in list pat_list */
void pattern_match(list<Punit*> pat_list, char* data, char* real_data, char* end_of_data) {
  list<Punit*>::iterator it = pat_list.begin();
  ret_t* retu = new ret();
  retu->startp = data;
  int dist;
  int data_len = end_of_data - data;
  retu->len = data_len;
  retu->quick_ref = 0;
  char* match_start;
  char* start_of_data = data;
  int comb_mlen = 0;
  int l = 0;
  int number_c = 0;
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
        for (it = pat_list.begin(); it != pat_list.end(); it++) {
//          cout << "mlen: " << (*it)->mlen << "\n";
          comb_mlen += (*it)->mlen;
        }
        dist = (retu->startp - comb_mlen) - start_of_data;
        int punit_start = 0;
       
//        cout << "hej: " << retu->startp << "\n";
//        cout << "comb_mlen: " << comb_mlen << "\n";
//        cout << "end of match: " << x << "\n";
        number_c++;
        printf("%i  %.*s\n", dist, comb_mlen, real_data + dist);
        it = pat_list.begin();
        data = retu->startp;
        comb_mlen = 0;
        retu->len = data_len;
//        retu->len = end_of_data - data;
        it = pat_list.begin();
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
//        cout << number_c << "\n";
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
    size_t a = fread(&real_data[0], 1, real_data.size(), fp);
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
  list<Punit*> pat_list = parse(pats, data, end_of_data, end_of_data - data);
  if (pat_list.empty()) {
    return -1;
  }
  #define T_BIT 0x08;
  unsigned char x = T_BIT;
  unsigned char t = (unsigned char) (( x >> 4) & 15);
//  cout << (int) x << "\n";
//  cout << (int) t << "\n";
  pattern_match(pat_list, data, rdata, end_of_data);
  return 0;
}
