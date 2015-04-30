#ifndef PUNIT_H
#define PUNIT_H

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <list>
using namespace std;

extern char punit_to_code[256];
extern char punit_to_code_for_data[256];
extern char code_to_punit[256];

typedef struct ret {
  char* startp;
  int len;
  int quick_ref;
} ret_t;

// for loosely fitted patterns (inserts, deletions, mismatches)
struct stackent {
  char* p1;
  char* p2;
  int n1,n2;
  int mis,ins,del;
  int next_choice;
};

int build_conversion_tables(void);

class Punit {
  public:
    char* data_start;
    char* data_end;
    int data_len;
    int run_len;
    int run_len_s;
    int mlen;
    char* mlen_start;
    int is_amb;
    int quick_ref;
    int strech;
    char* code;
    char* prev;
    char* orig_code;
    //Loose match saved variables
    char* p1;
    char* p2;
    Punit(char* data_s, char* data_e, int data_l, char* c);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual ret_t* search(ret_t* retu);
    virtual void reset(void);
    char known_char(char C);

    /* Match 2 given bases with eachother using the converted bit type*/
    bool matches(char C1, char C2);
};

typedef struct var {
  string name;
  Punit* var_punit;
  Punit* nxt_punit;
  int first;
} var_t;

/* punit range inherites from punit, is used to jump in the data e.g
*  6..8 jumps from 6 to 8 charecters
*  in the data */
class Range: public Punit {
  public:
    int len;
    int width;
    int inc_width; //If range is called with a width, this is used
    Range(char* data_s, char* data_e, int data_l, int le, char* c, int w);
    void reset(void);
    ret_t* search(ret_t* retu);
};

/* punit exact inherites from punit, is used to search for a litteral
*  in the data */
class Exact: public Punit {
    char* p1;
    char* p2;
    char* code_s;
    int i, nxtent, one_len, two_len, del_nxt, ins_nxt, len_s;
    //Integer array holding all tested match lengths
    int match_lens [1000];
    //list of the found match lengths
    char* match_list [1000];
    int match_list_len;
    //Stack with all matches
    stackent* match_stack;
    int found_matches;
  public:
    int len;
    int ins;
    int del;
    int mis;
    int flex;
    /* current insertions, deletions, and mismatches */
    int c_ins;
    int c_del;
    int c_mis;
    int c_flex;
    stackent* stack;
    char* tmp;
    Range* variable;
    char* cCode; //complementary code
    int comp;
    ret_t* loose_match(ret_t* retu, ret_t* new_retu);

    void stack_next(stackent* st,int nxtE, int N, 
                       char* p1, char* d1, int one_len,
                       int two_len);

    void pop(stackent* st, int nxtent, 
                    char** pattern, char** data, int* p_len, int* d_len, 
                    int* p_mis, int* p_ins, int* p_del,
                    int* ins_nxt_p, int* del_nxt_p);
    /*
    * Match: saves a match
    */
    void match();

    Exact(char* data_s, char* data_e, int data_len, int le, 
          char* c, int m, int d, int i, int f);

    void reset(void);

    ret_t* search(ret_t* retu);
};


class Reference: public Exact {
  public:
    int complement;
    int counter1;
    int counter2;
    char* var_old_prev; // To test if code needs to be made complementary again
    Punit* next_Punit;
    char* next_p_old_prev; // To test if code needs to be made complementary again
    int first_ref;
    Reference(char* data_s, char* data_e, int data_len, int comp, Range* var_p, 
              Punit* next_p, int first, int mis, int del, int ins, int flex);
    ret_t* search(ret_t* retu);
};

#endif
