#ifndef PUNIT_H
#define PUNIT_H

/*************************************************************
*                       Punit.h
* Description : Contains the skeleton of the PUs used in
*               scanfm
* Classes     : Punit
*               Sequence
*               Range
*               Reference
*************************************************************/

/*************************************************************
*                     Included Files
*************************************************************/
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <list>

using namespace std;

/*************************************************************
*                  Base/char conversion
*************************************************************/
extern char punit_to_code[256];
extern char punit_to_code_for_data[256];
extern char code_to_punit[256];

/*************************************************************
*                        Structs
*************************************************************/
/*************************************************************
* Struct      : PU_return
* Description : Used for holding the variables when a search 
*               returns from a PU to the over all loop.
* Variables   : startp = start position in the data
*               len = length allowed to move in the data from
*                     the start position
*               quick_ref = flag if a reference is right after
*                           being set.
*               opt = optimize flag
*               bst = flag if optimal PU
*************************************************************/
typedef struct PU_return {
  char* startp;
  int len;
  int quick_ref;
  int opt;
  int bst;
} PU_return_t;

/*************************************************************
* Struct      : stackent
* Description : Holds a instance when searching for matches
*               with MIDs
* Variables   : s_code = code string position
*               s_data = data position
*               s_code_len = length left in code
*               s_data_len = length left in data
*               mis, ins, del = MIDs
*               next_choice = Whether the next MID is a mismatchs,
*               a deletion or a insertion
*************************************************************/
struct stackent {
  char* s_code;
  char* s_data;
  int s_code_len, s_data_len;
  int mis,ins,del;
  int next_choice;
};

/*************************************************************
*                      Utility functions
*************************************************************/
int build_conversion_tables(void);

/*************************************************************
*                      Class descriptions
*************************************************************/
/*************************************************************
* Class       : Punit
* Description : The parrent class for all PUs
* Functions   : search
*               reset
*               get_score
*               get_min_len
*               get_max_len
*               known_char
*               matches
*************************************************************/
class Punit {
  public:
    int type;
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
    char* code_pos;
    char* data_pos;
    Punit(char* data_s, char* data_e, int data_l, char* c, int typ);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual PU_return_t* search(PU_return_t* PU_ret);
    virtual void reset(void);
    virtual int get_score();
    virtual int get_min_len();
    virtual int get_max_len();
    char known_char(char C);

    /* Match 2 given bases with eachother using the converted bit type*/
    bool matches(char C1, char C2);
};
/*************************************************************
* Struct      : var
* Description : variable is used for reference in order to save
*               the PUs that the reference PU needs information
*               from.
* Variables   : name = name of the saved variable (user written
*                      name to use when referencing)
*               var_punit = the PU the reference reffers to
*               nxt_punit = the PU after the refered PU
*               first = NOT USED
*************************************************************/
typedef struct var {
  string name;
  Punit* var_punit;
  Punit* nxt_punit;
  int first;
} var_t;

/*************************************************************
* Class       : Range
* Description : Range inserts a space between to pattern units
*               or is refered to by reference.
*************************************************************/
class Range: public Punit {
  public:
    int len;
    int width;
    int inc_width; //If range is called with a width, this is used
    int opti;
    Range(char* data_s, char* data_e, int data_l, int le, char* c, int w, int type);
    void reset(void);
    PU_return_t* search(PU_return_t* PU_ret);
    int get_score();
    int get_min_len();
    int get_max_len();
};

/*************************************************************
* Class       : Sequence
* Description : The sequence PU searches for a string in the
*               data, the string can be with mismatches, 
*               insertions and deletions, or complementary.
* Functions   : fuzzy_match
*               stack_next
*               start_pop
*************************************************************/
class Sequence: public Punit {
    char* code_pos;
    char* data_pos;
    char* p3;
    char* p4;
    char* code_s;
    int i, nxtent, code_len, data_left, del_nxt, ins_nxt;
    //Integer array holding all tested match lengths
    int match_lens[1000];
    //list of the found match lengths
    char* match_list[1000];
    int match_list_len;
    //Stack with all matches
    stackent* match_stack;
    int found_matches;
  public:
    int len, len_s;
    int ins;
    int del;
    int mis;
    int flex;
    char* prev_s;
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
    int opti;
    int best;
    PU_return_t* fuzzy_match(PU_return_t* PU_ret, PU_return_t* new_PU_ret);
    void stack_next(stackent* st,int nxtE, int N, 
                       char* code_pos, char* data, int code_len,
                       int data_left);

    void pop(stackent* st, int nxtent, 
                    char** pattern, char** data, int* p_len, int* d_len, 
                    int* p_mis, int* p_ins, int* p_del,
                    int* ins_nxt_p, int* del_nxt_p);
    Sequence(char* data_s, char* data_e, int data_len, int le, char* c, 
          int m, int d, int i, int f, int type);
    void match();

    void reset(void);

    PU_return_t* search(PU_return_t* PU_ret);
    int get_score();
    int get_min_len();
    int get_max_len();
};


/*************************************************************
* Class       : Reference
* Description : The reference PU searches for a match given
*               a ealier PU, in order to find either the earlier
*               PU again, a approksimation in the form of MIDs,
*               or the complementary of the earlier PU.
*************************************************************/
class Reference: public Sequence {
  public:
    int complement;
    int counter1;
    int counter2;
    char* var_old_prev; // To test if code needs to be made complementary again
    Punit* next_Punit;
    char* next_p_old_prev; // To test if code needs to be made complementary again
    int first_ref;
    Reference(char* data_s, char* data_e, int data_len, int comp, Range* var_p, 
              Punit* next_p, int first, int mis, int del, int ins, int flex, int type);
    PU_return_t* search(PU_return_t* PU_ret);
    int get_score();
    int get_min_len();
    int get_max_len();
};

#endif
