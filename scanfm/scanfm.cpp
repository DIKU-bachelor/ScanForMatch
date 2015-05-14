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
#include <time.h>
using namespace std;

#define fsize 250000000
#define patsize 1000

// Splits text into substrings by delimiter del and returns a list of these
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

  // Removing unwanted characters from last punit
  for (int i = 0; i < text.length(); i++) {
    if (text[i] == '\n' || text[i] == '\r' || text[i] == '\t' || text[i] == '\0' 
      || text[i] == ' ') {
      text.erase(i, 1);
    }
  }
  pat_list.push_back(text);
  return pat_list;
}


// Parses text to find punits and returns a list of these
list<Punit*> parse(string text, char* start_of_data, char* end_of_data) {

  // pat_list: contains the parsed punits to be returned
  list<Punit*> pat_list;

  // var_list: contains structs of variable name, pointer Range punit where variable is being set and
  // pointer to next punit. Used to link References punits to Range punits
  list<var_t*> var_list;
  list<var_t*>::iterator var_it;
  int save_next = 0;
  Punit* var_p;
  Punit* var_nxt_p;
  var_t* va_tmp;
  string var_name;

  // Variables used for converting Exact punits into 4-BIT charfields
  char x[] = {'A','C','G','T','U','M','R','W','S','Y','K','B','D','H','V','N'};
  list<char> known_chars (x, x + 16);
  char* conv_code;

  int data_len = end_of_data - start_of_data;
  int comp;
  int eq;
  int dots;
  string range_min_s;
  string range_max_s;
  int range_min;
  int range_max;
  int let_count;
  int until_brac;
  char* temp_p;
  int brac;
  int ex_len;
  int com;
  string mis_s;
  string ins_s;
  string del_s;
  int complem;
  int mis;
  int ins;
  int del;
  int inval_int;
  int first_ref = 0;

  list<string> split_text = split_str(text, ' ');

  // Looping over possible punits, parsing and adding to pat_list
  for (list<string>::iterator it = split_text.begin(); it != split_text.end(); it++) {
    string pu = (*it);
    eq = (*it).find('=');

    // Variable type punit, saves variable name in var_list
    if (eq != string::npos) {
      var_name = (*it).substr(0, eq);
      pu = pu.substr(eq + 1);
    }
    dots = pu.find("...");

    // Range type punit, verifies that both sides of ... are valid digits
    if (dots != string::npos) {
      range_min_s = pu.substr(0, dots);
      range_max_s = pu.substr(dots + 3);
      for (int i = 0; i < range_min_s.length(); i++) {
        if ((! isdigit(range_min_s[i])) || (! isdigit(range_max_s[i]))) {
          cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1 
            << ": Range type punit must be in format <int>...<int>\n";
          pat_list.clear();
          return pat_list;
        }
      }
      range_min = atoi(range_min_s.c_str());
      range_max = atoi(range_max_s.c_str());
      Range* ra = new Range(start_of_data, end_of_data, data_len, range_min, NULL, range_max - range_min, 2);
      pat_list.push_back(ra);

      // If previous punit was being set as variable
      if (save_next == 1) {
        va_tmp->nxt_punit = ra;
        save_next = 0;
      }

      // If this punit is a variable being set
      if (pu.length() != (*it).length()) {
        va_tmp = new var();
        va_tmp->name = var_name;
        va_tmp->var_punit = ra;
        va_tmp->first = 1;
        var_list.push_back(va_tmp);
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

    // Exact type punit, checks validity of each letter and converts to 4-BIT charfields
    let_count = 0;
    for (int i = 0; i < pu.length(); i++) {
      if (find(known_chars.begin(), known_chars.end(), toupper(pu[i])) == known_chars.end()) {
        break;
      }
      let_count++;
    }
    until_brac = pu.find('[');
    if (until_brac == string::npos) {
      until_brac = pu.length();
    }
    if (let_count == until_brac) {
      conv_code = new char[1000];
      temp_p = new char[until_brac + 1];
      strcpy(temp_p, pu.c_str());
      for(int i = 0; i < until_brac; i++){
        conv_code[i] = punit_to_code[temp_p[i]];
      }
      conv_code[pu.length()] = '\0';
    }

    // ex_len is used to determine length of Exact punit before the [ char
    brac = pu.find('[');
    ex_len = until_brac;
    if (brac != string::npos) {
      ex_len = brac;
    }

    // Reference type punit, finds potential ~ and searches var_list to determine validity of reference
    if (let_count != ex_len) {
      comp = pu.find('~');
      if (comp != string::npos) {
        pu = pu.substr(1);
      }
      brac = pu.find('[');
      string until_brac = pu;
      if (brac != string::npos) {
        until_brac = pu.substr(0, brac);
      }

      // Searches for valid variable in var_list
      for (var_it = var_list.begin(); var_it != var_list.end(); var_it++) {
        if (until_brac.compare((*var_it)->name) == 0) {
          var_p = (*var_it)->var_punit;
          var_nxt_p = (*var_it)->nxt_punit;
          if ((*var_it)->first == 1) {
            first_ref = 1;
            (*var_it)->first = 0;
          }
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
    mis_s;
    ins_s;
    del_s;
    mis = 0;
    ins = 0;
    del = 0;
    brac = pu.find('[');

    // Finds parses number of mismatches, insertions and deletions
    if (brac != string::npos) {
      pu = pu.substr(brac + 1);
      com = pu.find(',');
      if (com != string::npos) {
        ins_s = pu.substr(0, com);
        pu = pu.substr(com + 1);
      } else {
        cout << "ERROR: Could not parse punit " << distance(split_text.begin(), it) + 1
          << ": Mismatches, insertions and deletions must be declared in format:\n<punit>[?,?,?]\n";
          pat_list.clear();
          return pat_list;
      }
      com = pu.find(',');
      if (com != string::npos) {
        mis_s = pu.substr(0, com);
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
      inval_int = 0;
      for (int i = 0; i < mis_s.length(); i++) {
        if (! isdigit(mis_s[i])) {
          inval_int = 1;
        }
      }
      for (int i = 0; i < ins_s.length(); i++) {
        if (! isdigit(ins_s[i])) {
          inval_int = 1;
        }
      }
      for (int i = 0; i < del_s.length(); i++) {
        if (! isdigit(del_s[i])) {
          inval_int = 1;
        }
      }
      if (inval_int) {
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
    if (ex_len == let_count) {
      Exact* ex = new Exact(start_of_data, end_of_data, data_len, (int) until_brac, 
        conv_code, mis, del, ins, 0, 1);
      pat_list.push_back(ex);
      if (save_next == 1) {
        va_tmp->nxt_punit = ex;
        save_next = 0;
      }
      continue;
    } 
    if (var_it != var_list.end()) {
      complem = 0;
      if (comp != string::npos) {
        complem = 1;
      }
      Reference* re = new Reference(start_of_data, end_of_data, data_len, complem, (Range*)
        var_p, var_nxt_p, first_ref, mis, del, ins, 0, 3);
      pat_list.push_back(re);
      first_ref = 0;
      if (! re->next_Punit) {
        re->next_Punit = re;
      }
      if (save_next == 1) {
        va_tmp->nxt_punit = re;
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

typedef struct opti_info {
  int opt_index;
  int min_start_dist;
  int max_start_dist;
} opti_info_t;

opti_info_t* find_optimal(list<Punit*> pat_list) {
  list<Punit*>::iterator it;
  opti_info_t* opt = new opti_info();
  int index = 0;
  int best_index = 0;
  int score = 0;
  int cur_score = 0;
  for (it = pat_list.begin(); it != pat_list.end(); it++) {
    cur_score = (*it)->get_score();
    if (score < cur_score) {
      best_index = index;
      score = cur_score;
    }
    index++;
  }
  index = 0;
  for (it = pat_list.begin(); it != pat_list.end(); it++) {
    if (index++ >= best_index) {
      break;
    }
    opt->min_start_dist += (*it)->get_min_len();
    opt->max_start_dist += (*it)->get_max_len();
  }
  opt->opt_index = best_index;
  return opt;
}



void pattern_match_opti(list<Punit*> pat_list, char* data, char* real_data, char* end_of_data, opti_info_t* opt) {
  list<Punit*>::iterator it = pat_list.begin();

  int data_len = end_of_data - data;
  char* start_of_data = data;

  // Initializing return package that is passed and returned between punits
  ret_t* retu = new ret();
  retu->startp = data;
  retu->len = data_len;
  retu->quick_ref = 0;

  int dist_to_match;
  int prev_dist_to_match = 0;
  int prev_comb_mlen = 0;
  int comb_mlen = 0;
  char* best_prev;
  retu->bst = 1;
  advance(it, opt->opt_index);
  retu->len = data_len;
//  cout << opt->max_start_dist << "\n";
//  cout << opt->min_start_dist << "\n";
  // Each iteration in loop finds optimal pu and tries to match PU's around it
  while (true) {
//    cout << retu->startp << "\n";
//    cout << "search best\n";
    retu = (*it)->search(retu);
    if (retu->startp) { 
      if ((*it)->prev - data <= opt->max_start_dist) {
        retu->startp = data;
      } else {
        retu->startp = (*it)->prev - opt->max_start_dist;
      }
//      cout << opt->max_start_dist << "\n";
      retu->len = opt->max_start_dist - opt->min_start_dist;
      retu->quick_ref = 0;
      retu->opt = 1;
      best_prev = (*it)->prev;
      it = pat_list.begin();
//      cout << "start pos to pass: " <<  retu->startp << "\n";
//      cout << "BEFORE normal search\n";
//      cout << "distance to startp: " << (retu->startp - start_of_data) << "\n";
      // Now we match every PU around the found one
      while (true) {
        retu = (*it)->search(retu);
        if (retu->startp) {
//          cout << "PU match!!\n";
          // If whole pattern matched
          if (++it == pat_list.end()) {
            for (it = pat_list.begin(); it != pat_list.end(); it++) {
              comb_mlen += (*it)->mlen;
//              cout << "MLEN: " << (*it)->mlen << "\n";
    //          printf("%p\n",(*it)->prev);
            }
            dist_to_match = (retu->startp - comb_mlen) - start_of_data;
            if (dist_to_match - prev_dist_to_match >= prev_comb_mlen) {
              printf("%i  %.*s\n", dist_to_match + 1, comb_mlen, real_data + dist_to_match);
            }
            prev_comb_mlen = comb_mlen;
            prev_dist_to_match = dist_to_match;
            comb_mlen = 0;
            it = pat_list.begin();
            advance(it, opt->opt_index);
            retu->len = data_len;
            retu->startp = best_prev + 1;
            break;
          }
          continue;
        } else {
//          cout << "PU not match...\n";
          if (it == pat_list.begin()) {
            advance(it, opt->opt_index);
            retu->len = data_len;
//            cout << "BEFORE INCREMENT: " << (*it)->prev << "\n";
            retu->startp = best_prev + 1;
//            cout << "Counter: " << (*it)->prev - start_of_data << "\n";
//            cout << "Before break\n";
            break;
          }
          it--;
        }
      }
    } else {
      return;
    }
  }
}





// Looks through the data string to find pattern specified in list pat_list
void pattern_match(list<Punit*> pat_list, char* data, char* real_data, char* end_of_data) {
  list<Punit*>::iterator it = pat_list.begin();

  int data_len = end_of_data - data;
  char* start_of_data = data;

  // Initializing return package that is passed and returned between punits
  ret_t* retu = new ret();
  retu->startp = data;
  retu->len = data_len;
  retu->quick_ref = 0;

  int dist_to_match;
  int comb_mlen = 0;

  while (true) {
    if (it == pat_list.begin()) {
      retu->len = data_len;
    }
    retu = (*it)->search(retu);
    //printf("retu startp = %p \n", retu->startp);
    // If the punit matched
    if (retu->startp) {
      // If the whole pattern matched
      if (++it == pat_list.end()) {
        for (it = pat_list.begin(); it != pat_list.end(); it++) {
          comb_mlen += (*it)->mlen;
          cout << (*it)->mlen << "\n";
//          printf("%p\n",(*it)->prev);
        }
        dist_to_match = (retu->startp - comb_mlen) - start_of_data;
        printf("%i  %.*s\n", dist_to_match + 1, comb_mlen, real_data + dist_to_match);
        it = pat_list.begin();
        data = retu->startp;
        comb_mlen = 0;
        retu->len = data_len;
        it = pat_list.begin();
        continue;
      }
      data = retu->startp;
      continue;

    // If the punit didn't match
    } else {
      // If whole pattern didn't match
      if (it == pat_list.begin()) {
        return;
      }
      it--;
    }
  }
}

int main(int argc, char* argv[]) {
  clock_t tim = clock();
  build_conversion_tables(); 
  if (argc == 1) {
    cout << "ERROR: call to scanfm need arguments in format:\nscanfm -p <pattern> -d <datafile>\n";
    return -1;
  }
  int arg = 0;

  // Skip until -p flag
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

  // Read all of the pattern
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
    data[i] = punit_to_code_for_data[tolower(real_data[i])];
    end_of_data++;
  }
  data[++i] = '\0';

  list<Punit*> pat_list = parse(pats, data, end_of_data);

  // If an error occured during parsing
  if (pat_list.empty()) {
    return -1;
  }
  opti_info_t* op = find_optimal(pat_list);
  pattern_match_opti(pat_list, data, rdata, end_of_data, op);
//  pattern_match(pat_list, data, rdata, end_of_data);
  tim = clock() - tim;
  printf("Total time: %f seconds\n", ((float) tim) / CLOCKS_PER_SEC);
  return 0;
}
