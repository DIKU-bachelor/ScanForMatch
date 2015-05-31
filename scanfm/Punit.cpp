/*************************************************************
*                        Punit.cpp
* Description : This file contains all class method 
*               declarations.
* Classes     : Punit
*                 -Punit
*                 -Search
*                 -get_score
*                 -get_min_len
*                 -get_max_len
*                 -known_char
*                 -matches
*               Sequence
*                 -Sequence
*                 -Search
*                 -fuzzy_match
*                 -stack_next
*                 -stack_pop
*                 -match
*                 -get_score
*                 -get_min_len
*                 -get_max_len
*               Range
*                 -Range
*                 -Search
*                 -get_score
*                 -get_min_len
*                 -get_max_len
*               Reference 
*                 -Reference
*                 -Search
*                 -get_score
*                 -get_min_len
*                 -get_max_len
* Utility functions :
*               build_conversion_tables
*               pprint
*
*************************************************************/


/*************************************************************
*                     Included Files
*************************************************************/

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Punit.h"
using namespace std;


/*************************************************************
*                  Base/char conversion
*************************************************************/
int conv_table_init = 1;
char punit_to_code[256];
char punit_to_code_for_data[256];
char code_to_punit[256];


/*************************************************************
*                     Allowed Bases
*************************************************************/
bool known_char_i[16] = {0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};
char known_char_index[16] =
                 {-1,0,1,-1,2,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1};


/*************************************************************
*                        Macros
*************************************************************/
#define A_BIT 0x01              /* char bitfield */
#define C_BIT 0x02              /* char bitfield */
#define G_BIT 0x04              /* char bitfield */
#define T_BIT 0x08              /* char bitfield */
#define NO_BIT 0x00

#define max_var_size 10000

/*************************************************************
*                     Utility functions
*************************************************************/
void pprint(char* text, int len) {
  for (int i = 0; i < len; i++) {
    cout << code_to_punit[text[i]];
  }
  cout << "\n";
}

/*************************************************************
*                     Class Methods
*************************************************************/
/*************************************************************
*                     Class Punit
*************************************************************/
/*************************************************************
* Function    : Punit
* Description : Initiates the varibles needed for all pattern
                units.
* Parameters  : data_s = Data start position
                data_e = Data end position
                data_l = Data length
                c      = Pattern bases (code)
*************************************************************/
Punit::Punit(char* data_s, char* data_e, int data_l, char* c, int typ){
    data_start = data_s;
    data_end = data_e;
    data_len = data_l;
    mlen = 0;
    strech = 0;
    code = c;
    type = typ;
}

/************************************************************
* Function    : get_score
* Description : Abstract function to return the optimization
*               score of the PU
* Returns     : PU optimization score
************************************************************/
int Punit::get_score() {
  cout << "Punit get_flex called, should not happen...\n";
  return 0;
}

/************************************************************
* Function    : get_min_len
* Description : Abstract function to return the minimum length
*               the pattern fills in the data.
* Returns     : PU minimum length
************************************************************/
int Punit::get_min_len() {
  cout << "Punit get_min_len called, should not happen...\n";
  return 0;
}

/************************************************************
* Function    : get_max_len
* Description : Abstract function to return the maximum length
*               the pattern fills in the data.
* Returns     : PU maximum length
************************************************************/
int Punit::get_max_len() {
  cout << "Punit get_max_len called, should not happen...\n";
  return 0;
}

/**
* NOT USED!!!!! reset pattern unit
* resets all needed variables in a pattern unit
*/
void Punit::reset(void) {}

/************************************************************
* Function    : Punit::search
* Description : Abstract function for pattern unit search.
*               This is the main function of the pattern unit
*               it is called to check if a certain match fits
*               from the start position given, with a given
*               length to expand in the data.
* Returns     : PU_ret = In the case of a match holds the 
*                        information for start position for
*                        the next PU, and the length it is
*                        allowed to expand in the data.
************************************************************/
PU_return_t* Punit::search(PU_return_t* start) {
  cout << "Punit search called, should not happen...\n";
}

/************************************************************
* Function    : known_char
* Description : Checks if the base being given in the data, 
*               can match with a pattern base.
* Returns     : Returns true if the base can match, and false
*               if it can't.
************************************************************/
char Punit::known_char(char C) {
    return (known_char_i[(unsigned char)C]);
}

/************************************************************
* Function    : matches
* Description : Checks if a 2 bases match.
* Returns     : True if they match false if they don't
************************************************************/
bool Punit::matches(char C1, char C2) {
     return (known_char((C1) & 15) && ((((C1) & 15) & ((C2) & 15)) == 
            (C1 & 15)));
}

/************************************************************
*                 Class Sequence
************************************************************/

/************************************************************
* Function    : Sequence
* Description : Constructor for the Sequence PU. Variables 
*               are initiated and stacks are allocated for
*               mismatches, deletions and insertions. 
* Parameters  : data_s, data_e, data_len, c are described in
*               the Punit constructor.
*               m, d, i = mismatches, deletions and insertions
*               type = describes the PU type
*               le = length of the PU's sequence (code)
************************************************************/
Sequence::Sequence(char* data_s, char* data_e, int data_len, int le, char* c, 
             int m, int d, int i, int f, int type) : Punit(data_s, data_e, data_len, c, type){
    len = le;
    ins = i;
    del = d;
    mis = m;
    flex = f;
    c_ins = del;
    c_del = ins;
    c_mis = mis;
    c_flex = flex;
    stack = (stackent*)malloc(sizeof(stackent)*10000000);
    match_stack = (stackent*)malloc(sizeof(stackent)*10000000);
    for(i = 0; i < 1000; i++){
      match_lens[i] = 0;
    }
    found_matches = 0;
    match_list_len = -1;
}

/************************************************************
*                Sequence stack functions
************************************************************/
/************************************************************
* Function    : stack_next
* Description : Saves a instance of the algorithm, when 
*               searching for a match with, mismatches, deletions
*               and insertions.
* Parameters  : st = The stack the instance is saved in
*               nxtE = next save spot in the stack
*               N = next choice for MID
*               code_pos = current position in the pattern sequence
*               data = current position in the data
*               code_len = length left of the pattern sequence
*               data_left = length left of the data
************************************************************/
void Sequence::stack_next(stackent* st,int st_nxtent, int st_next_choice, 
                       char* st_code_pos, char* st_data, int st_code_len,
                       int st_data_left) {
  st[st_nxtent].s_code=st_code_pos; 
  st[st_nxtent].s_data=st_data; 
  st[st_nxtent].s_code_len=st_code_len;
  st[st_nxtent].s_data_len=st_data_left;
  st[st_nxtent].mis=c_mis;
  st[st_nxtent].ins=c_ins; 
  st[st_nxtent].del=c_del; 
  st[nxtent++].next_choice=st_next_choice;
}

/************************************************************
* Function    : pop
* Description : Extracts the last instance in the stack, when
*               searcing for a match with MID
* Parameters  : st = The stack the instance is saved in
*               nxtent = next save spot in the stack
*               pattern = current position in the pattern sequence
*               data = current position in the data
*               p_mis, p_del, p_ins = mismatches, insertion and 
*               deletions
*               p_len = pattern length
*               d_len = data length
************************************************************/
void Sequence::pop(stackent* st, int nxtent, 
         char** pattern, char** data, int* p_len, int* d_len, 
         int* p_mis, int* p_ins, int* p_del,
         int* ins_nxt_p, int* del_nxt_p){
  *pattern = st[nxtent].s_code;
  *data = st[nxtent].s_data;
  *p_len = st[nxtent].s_code_len;
  *d_len = st[nxtent].s_data_len;
  *p_mis = st[nxtent].mis;
  *p_ins = st[nxtent].ins;
  *p_del = st[nxtent].del;

  //handle where to go next
  if (stack[nxtent].next_choice == 1) {
    if (*p_del){
      st[nxtent].next_choice = 2;
    }
    *ins_nxt_p = 1;
  }else{
    *del_nxt_p = 1;
  }
}


/************************************************************
* Function    : match
* Description : Stores a found match with MIDs.
************************************************************/
void Sequence::match(){
  match_stack[found_matches].s_data=data_pos;
  match_stack[found_matches].mis=c_mis;
  match_stack[found_matches].ins=c_ins; 
  match_stack[found_matches++].del=c_del;
}


/************************************************************
* Function    : Sequence::search
* Description : Checks if a match can be found given; a
*               sequence of bases, a position in the data, 
*               and flags, if the search includes MIDs or
*               is complementary or is a reference that has
*               just been set.
* Parameters  : PU_ret = Start position in the data for the 
*               check and the length able to be moved in the
*               data.
* Returns     : Null if no match is found, or the end position
*               of the match found, and the length the next
*               indent in the data.
************************************************************/

PU_return_t* Sequence::search(PU_return_t* PU_ret){
  //If not backtracking
  if(PU_ret->startp != NULL) {
    // Reset varibels search at a new position
    if (PU_ret->opt) {
      opti = 1;
      PU_ret->opt = 0;
    }
    prev = PU_ret->startp;
    len_s = len;
    code_s = code;
    prev_s = prev;
    run_len = PU_ret->len;
    run_len_s = run_len;
    code_len = len;
    data_left = data_end-prev;
    del_nxt = 0;
    ins_nxt = 0;
    mlen = 0;
    code_pos = code;
    data_pos = prev;
    flex = ins + del + mis;
    mlen_start = PU_ret->startp;
  // Backtracking
  } else {
    mlen = 0;
  }
  int first = 0;
  if (run_len_s == data_len) {
    first = 1;
  }
/*********************************************************
*               Exact sequence
*********************************************************/
  if(flex == 0){
    if (code_len > data_left) {
      PU_ret->startp = NULL;
      return PU_ret;
    }
    int i;
    int c = 0;
    prev_s = prev;
    if (PU_ret->startp == NULL && run_len >= 0) {
      prev++;
    }
    /* Search loops 
    *  There are multiple ways, references and sequences can search for a exact match
    *  these cases are split up in 4 categories.
    *  with or withour complementary
    *  with or without the previous PU being a varible being set for a reference
    */
    if (quick_ref) {
      if (comp) {
      // Complementary, Quick reference (the privious PU was the varible being set)
        p3 = code + len + (run_len_s - run_len) - 1;
        while (prev_s <= prev_s + run_len-- && prev + len + (prev - prev_s) < data_end) {
          if (matches(*prev,(((*p3++) >> 4) & 15))){
            data_pos = prev+1;p3 = p3 - 2;
            c = prev - prev_s;
            for (i = (len + c) - 1; i && matches(*data_pos,(((*p3) >> 4) & 15)); i--,p3--,data_pos++)
              ;
            if (!i){
              c = prev - prev_s;
              len += c;
              mlen = len;
              break;
            }
          }
          p3 = code + len + (run_len_s - run_len) - 1;
          prev++;
        }

      // Quick reference (the privious PU was the varible being set)
      } else {
        while (prev_s <= prev_s + run_len-- && prev + len + (prev - prev_s) < data_end) {
          if (matches(*prev,*code)){
            data_pos = prev+1; p3 = code+1;
            c = prev - prev_s;
            for (i = (len + c) - 1; i && matches(*data_pos,*p3); i--,p3++,data_pos++)
              ;
            if (!i){
              c = prev - prev_s;
              len += c;
              mlen = len;
              break;
            }
          }
          prev++;
        }
      }
    } else {
    // Complementary
      if (comp) {
        p3 = code + len - 1;
        while (0 <= run_len-- && data_end - (prev + len) > 0) {          
          if (matches(*prev,((*(p3) >> 4) & 15))){
            data_pos = prev+1; p4 = p3 - 1;
            for (i = len - 1; i && matches(*data_pos,(((*p4) >> 4) & 15)); i--,p4--,data_pos++)
              ;
            if (!i){
              c = prev - prev_s;
              mlen = len;
              break;
            }
          }
          prev++;
        }
      } else {
      // Normal case.
        while (prev_s <= prev_s + run_len-- && prev + len < data_end) {
          if (matches(*prev,*code)){
            data_pos = prev+1;p3 = code+1;
            if (*p3 == '\0') {
              c = prev - prev_s;
              mlen = len;
              break;
            }
            for (i=len-1; i && matches(*data_pos,*p3); i--,p3++,data_pos++)
              ;
            if (!i){
              c = prev - prev_s;
              mlen = len;
              break;
            }
          }
          prev++;
        }
      }
    }
    PU_ret->quick_ref = 0;
    PU_ret->len = 0;
    // If we have a match
    if(len == mlen){
      if ((first != 1) && (opti != 1)) {
        mlen += c;
        mlen = (prev + len) - mlen_start;
      }
      PU_ret->startp = (prev + len);
      /* Both the variable and the reference is c longer, 
         so mlen is incremented twice with c */
      if (PU_ret->quick_ref) {
        mlen += c;
      }
      return PU_ret;
    } else {
      PU_ret->startp = NULL;
      return PU_ret;
    }
/********************************************************************
*                Fuzzy matching sequence
*******************************************************************/
  } else {
    int i;
    int c = 0;
    PU_return_t* new_PU_ret = new PU_return_t;
    if (PU_ret->bst > 0) {
      match_list_len = 0;
    }
    if (quick_ref) {
      if (comp) {
        // If this punit is a complementary and previous punit is the variable being set
        if (match_list_len > 0) {
          new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
          mlen = mlen + (prev - prev_s);
          return new_PU_ret;
        }
        while (0 <= run_len && prev + len + (prev - prev_s) < data_end) {
          code = code_s + run_len;
          len = len_s + (prev - prev_s);
          new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
          if(new_PU_ret->startp){
            mlen = mlen + (prev - prev_s);
            return new_PU_ret;
          }
          PU_ret->startp = ++prev; data_pos++; run_len--;
        }

      // If this punit is a non-complementary and previous punit is the variable being set
      } else {
        if (match_list_len > 0) {
          new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
          mlen = mlen + (prev - prev_s);
          return new_PU_ret;
        }
        while (prev_s <= prev_s + run_len && 
               prev + len + (prev - prev_s) < data_end) {
          len = len_s + prev - prev_s;
          new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
          if(new_PU_ret->startp){
            mlen = mlen + prev - prev_s;
            return new_PU_ret;
          }
          PU_ret->startp = ++prev; data_pos++; run_len--;
        }
      }
    } else {
      if (match_list_len > 0) {
        new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
        return new_PU_ret;
      }
      // If this Punit is not complementary and the previous Punit was not the variable being set
      while (prev_s <= prev_s + run_len && prev + len < data_end) {
        new_PU_ret = fuzzy_match(PU_ret, new_PU_ret);
        if(new_PU_ret->startp){
          if ((! first) && (! opti)) {
            mlen = mlen + prev - prev_s;
          }
          return new_PU_ret;
        }
        PU_ret->startp = ++prev; data_pos++; run_len--;
      }
    }
  }
  PU_ret->startp = NULL;
  PU_ret->len = 0;
  return PU_ret;
}



/************************************************************
* Function    : fuzzy_match
* Description : Checks given a position in the data, a string
*               of bases, and the number of MIDs if a match
*               can be found.
*               In the case a match can be found, it finds
*               all lengths a match can be found with.
* Parameters  : PU_ret = Start position in the data for the 
*               check and the length able to be moved in the
*               data.
*               new_PU_ret : a return struct for use in 
*               calling function
* Returns     : Null if no match is found, or the end position
*               of a possible match. In the event of backtracking
*               it returns the next start position in the data
*               if more are possible.
************************************************************/
PU_return_t* Sequence::fuzzy_match (PU_return_t* PU_ret, PU_return_t* new_PU_ret){
  if(PU_ret->startp == NULL){     /* Backtracking */
    if(match_list_len >= 0){
      new_PU_ret->startp = (char*)(match_list[match_list_len--]);
      new_PU_ret->len = 0;
      mlen = new_PU_ret->startp - prev;
      return new_PU_ret;
    } else {
      new_PU_ret->startp = NULL;
      new_PU_ret->len = 0;
      return new_PU_ret;
    }
  }
  if (PU_ret->startp != NULL) {    /* New position */
    code_len = len;
    data_left = data_end-prev;
    del_nxt = 0;
    ins_nxt = 0;
    mlen = 0;
    code_pos = code;
    data_pos = prev;
    flex = ins + del + mis;
    c_mis = mis;
    c_ins = del;
    c_del = ins;
    nxtent = 0;
    found_matches = 0;
    memset(match_lens, 0, sizeof(match_lens));
    match_list_len = 0;
/********************************************************************
*                      Only mismatches
********************************************************************/
    if ((c_ins == 0) && (c_del == 0)){
      if (code_len > data_left)
      {
        new_PU_ret->startp = NULL;
        new_PU_ret->len = 0;
        return new_PU_ret;
      }
      int i;
      for (i=code_len; i >= 1; i--){
        if (!known_char((*data_pos)&15) ||
           (!matches(*data_pos, *code_pos) && (--c_mis < 0))){
          new_PU_ret->startp = NULL;
          new_PU_ret->len = 0;
          return new_PU_ret;
        } else {
            data_pos++; code_pos++;
        }
      }
      new_PU_ret->startp = (char*)(data_pos);
      new_PU_ret->len = 0;
      mlen  = (int) (data_pos - prev);
      return new_PU_ret;
    }
/********************************************************************
*               With insertions, and deletions.
********************************************************************/
    while (1){
      //Match
      if (!del_nxt && !ins_nxt 
          && (data_left && code_len >= 1 && known_char((*data_pos)&15) 
          && matches(*data_pos,*code_pos))){
        data_pos++; code_pos++; data_left--;
        if (!(--code_len)){
          match();
        }
      }
      // Mismatch
      else if (!del_nxt && !ins_nxt &&(c_mis && (code_len >= 1) && (data_left >= 1))){
        if (c_ins){
          stack_next(stack, nxtent, 1, code_pos, data_pos, code_len, data_left);
        }
        else if (c_del){
          stack_next(stack, nxtent, 2, code_pos, data_pos, code_len, data_left);
        }
        c_mis--; code_pos++; data_pos++; code_len--; data_left--;
        if (! code_len)
        {
          match();
        }
      }
      //Insertion
      else if (!del_nxt && (ins_nxt  || (c_ins) && (code_len >= 1))){
        if (!ins_nxt && (c_del) && (data_left >= 1)){
          stack_next(stack, nxtent, 2, code_pos, data_pos, code_len, data_left);
        }
        ins_nxt = 0;
        c_ins--; code_pos++; code_len--;
        if (! code_len){
          match();
        }
      }
      //Deletion
      else if (del_nxt || ((c_del) && (data_left >= 1))){
        del_nxt = 0;
        c_del--; data_pos++; data_left--;
        if (! code_len){
          match();
        }
      }
      else if (nxtent--){
        pop(stack, nxtent, 
         &code_pos, &data_pos, &code_len, &data_left, 
         &c_mis, &c_ins, &c_del,
         &ins_nxt, &del_nxt);
      } else{
        break; 
      }
    }
    int n;
    
/**************************************************************
*             Calculate Match lengths
**************************************************************/
    if(found_matches-- > 0){
      for(found_matches; found_matches >= 0; found_matches--){
        char* temp_code_pos = match_stack[found_matches].s_data;
        //char* temp_code_pos - n;// = temp_code_pos - n;
        for(n = match_stack[found_matches].ins; 
            n && temp_code_pos - n - prev > 0; 
            n--){
          if(!match_lens[temp_code_pos - n - prev]){
            match_lens[temp_code_pos - n - prev] = 1;
            match_list[match_list_len] = temp_code_pos - n;
            match_list_len++;
          }
        }
        //Move match len with deletions
        for(n = match_stack[found_matches].del; n; n--){
          if(!match_lens[temp_code_pos - prev + n]){
            match_lens[temp_code_pos - prev + n] = 1;
            match_list[match_list_len] = temp_code_pos + n;
            match_list_len++;
          }
        }
        // save the original match length
        if(!match_lens[temp_code_pos - prev]){
          match_lens[temp_code_pos - prev] = 1;
          match_list[match_list_len] = temp_code_pos;
          match_list_len++;
        } 
      }
      match_list_len--;
      new_PU_ret->startp = match_list[match_list_len--];
      new_PU_ret->len = 0;
      mlen = new_PU_ret->startp - prev;
      return new_PU_ret;
    }
  }
  new_PU_ret->startp = NULL;
  new_PU_ret->len = 0;
  return new_PU_ret;
}

/************************************************************
* Function    : reset (NOT USED)
* Description : Resets the PU so it is ready for a new start
*               position
************************************************************/
void Sequence::reset(void) {
  mlen = 0;
  c_ins = del;
  c_del = ins;
  c_mis = mis;
  c_flex = flex;
}

/************************************************************
* Function    : Sequence::get_score
* Description : Function to return the optimization
*               score of the PU
* Returns     : PU optimization score
************************************************************/
int Sequence::get_score() {
  return len - (mis + ins + del);
}

/************************************************************
* Function    : Sequence::get_min_len
* Description : Function to return the minimum length
*               the pattern fills in the data.
* Returns     : PU minimum length
************************************************************/
int Sequence::get_min_len() {
  return len - del;
}

/************************************************************
* Function    : Sequence::get_max_len
* Description : Function to return the maximum length
*               the pattern fills in the data.
* Returns     : PU maximum length
************************************************************/
int Sequence::get_max_len() {
  return len + ins;
}

/************************************************************
*                   Class Range
************************************************************/

/************************************************************
* Function    : Range::Range
* Description : The constructor to the Range PU, it allocates
*               space for the potentially used data segment
*               it jumps in the data.
*               And initiates its needed vairables.
************************************************************/
Range::Range(char* data_s, char* data_e, int data_l, int le, char* c, 
             int w, int type) : Punit(data_s, data_e, 0, c, type){
    len = le;
    width = w;
    data_len = data_l;
    code = new char[1000];
}



/************************************************************
* Function    : Range::search
* Description : Jumps a certain length in the data, and tells
*               the next PU how far it can search in the data
*               for a match.
* Parameters  : PU_ret = Start position in the data for the 
*               check and the length able to be moved in the
*               data.
* Returns     : Returns the end position of the match found, 
*               and the length the next PU can indent in the data.
************************************************************/
PU_return_t* Range::search(PU_return_t* PU_ret){
  int first = 0;
  if (PU_ret->len == data_len) {
    first = 1;
  }
  // The next punit will know that this is a variable being set
  if(PU_ret->startp == NULL) {
    // it can't backtrack any more
    if(inc_width == 0){
      return PU_ret;
    // backtracking one forward
    } else {
      inc_width--;
      strech++;
      if ((! first) && (! opti)) {
        mlen++;
      }
      prev++;
      if (prev + len < data_end) {
        PU_ret->len = width;
        PU_ret->startp = prev + len;
      }
      return PU_ret;
    }
  }
  mlen = len;
  strech = 0;
  prev = PU_ret->startp;
  if (PU_ret->opt) {
    opti = 1;
    PU_ret->opt = 0;
  }
  inc_width = PU_ret->len;
  if(PU_ret->startp + len < data_end){
    PU_ret->startp = (PU_ret->startp + len);
    PU_ret->len = width;
    mlen = len;
    return PU_ret;
  } else {
    PU_ret->startp = NULL;
    PU_ret->len = 0;
    return PU_ret;
  }
}
/**
* NOT USED!!! Range::Reset
* resets the variables of range
*/
void Range::reset(void) {
  mlen = 0;
}

/************************************************************
* Function    : Range::get_score
* Description : Function to return the optimization
*               score of the PU
* Returns     : PU optimization score
************************************************************/
int Range::get_score() {
  return 0;
}

/************************************************************
* Function    : Range::get_min_len
* Description : Function to return the minimum length
*               the pattern fills in the data.
* Returns     : PU minimum length
************************************************************/
int Range::get_min_len() {
  return len;
}

/************************************************************
* Function    : Range::get_max_len
* Description : Function to return the maximum length
*               the pattern fills in the data.
* Returns     : PU maximum length
************************************************************/
int Range::get_max_len() {
  return len + width;
}

/************************************************************
*                    Class Reference
************************************************************/


/************************************************************
* Function    : Reference::Reference
* Description : Initiates the needed variables for the reference
*               PU and allocates space for the saved substring.
* Parameters  : data_s, data_e, data_len is described in the 
*               PU constructor
*               comp = flag if it is complementary
*               var_p = points to the earlier referenced PU
*               nxt_p = PU after the referenced PU
*               mis, ins, del, flex are the fuzzy match vars
*               type = the type of the PU
************************************************************/
Reference::Reference(char* data_s, char* data_e, int data_len, int comp, Range* var_p, 
              Punit* nxt_p, int first, int mis, int ins, int del, int flex, int type)
           : Sequence(data_s, data_e, data_len, 0, NULL, mis, ins, del, flex, type) {
  variable = var_p;
  next_Punit = nxt_p;
  complement = comp;
  first_ref = first;
  cCode = new char[1000];
}

/************************************************************
* Function    : Reference::search
* Description : Uses the code string from a saved earlier PU
*               and either finds it at the position given, 
*               finds it complementary or with MIDs.
* Parameters  : PU_ret = Start position in the data for the 
*               check and the length able to be moved in the
*               data.
* Returns     : Returns the end position of the match found.
************************************************************/
PU_return_t* Reference::search(PU_return_t* PU_ret) {
  if (PU_ret->startp) {
    code = variable->prev;
    prev = PU_ret->startp;
    prev = PU_ret->startp;
    len = next_Punit->prev - variable->prev;
    // If this reference is immediately after the variable being set
    if (this == next_Punit) {
      quick_ref = 1;
    } else {
      quick_ref = 0;
    }
    comp = complement;
    if(complement && (ins != 0 || del != 0 || mis != 0)){
      if(quick_ref){
        len = variable->len;
        len_s = variable->len;
      }
      comp = complement;
      counter1 = len - 1;
      if (quick_ref) {
        counter1 += variable->width;
      }
      counter2 = 0;
      while(counter1 >= 0 && code + counter1 <= data_end){
        cCode[counter2++] = ((code[counter1--] >> 4) & 15);
      }
      code = cCode; 
    }
  }
  return Sequence::search(PU_ret);
}

/************************************************************
* Function    : Reference::get_score
* Description : Function to return the optimization
*               score of the PU
* Returns     : PU optimization score
************************************************************/
int Reference::get_score() {
  return 0;
}

/************************************************************
* Function    : Reference::get_min_len
* Description : Function to return the minimum length
*               the pattern fills in the data.
* Returns     : PU minimum length
************************************************************/
int Reference::get_min_len() {
  return variable->len - ins;
}

/************************************************************
* Function    : Reference::get_max_len
* Description : Function to return the maximum length
*               the pattern fills in the data.
* Returns     : PU maximum length
************************************************************/
int Reference::get_max_len() {
  return variable->len + variable->width + del;
}
/************************************************************
*                   Utility function
************************************************************/
/************************************************************
* Function    : build_convertion_tables
* Description : Creates the convertion tables to be used when
*               translating between bases, and chars.
* Returns     : returns 0 if succesfull.
************************************************************/
int build_conversion_tables()
{
    int the_char;

    for (the_char=0; the_char < 256; the_char++) {
        switch(tolower(the_char)) {
          case 'a': punit_to_code[the_char] = A_BIT;
            punit_to_code_for_data[the_char] = A_BIT;
            break;
          case 'c': punit_to_code[the_char] = C_BIT;
            punit_to_code_for_data[the_char] = C_BIT;
            break;
          case 'g': punit_to_code[the_char] = G_BIT;
            punit_to_code_for_data[the_char] = G_BIT;
            break;
          case 't': punit_to_code[the_char] = T_BIT;
            punit_to_code_for_data[the_char] = T_BIT;
            break;
          case 'u': punit_to_code[the_char] = T_BIT;
            punit_to_code_for_data[the_char] = NO_BIT;
          case 'm': punit_to_code[the_char] = (A_BIT | C_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'r': punit_to_code[the_char] = (A_BIT | G_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'w': punit_to_code[the_char] = (A_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 's': punit_to_code[the_char] = (C_BIT | G_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'y': punit_to_code[the_char] = (C_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'k': punit_to_code[the_char] = (G_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'b': punit_to_code[the_char] = (C_BIT | G_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'd': punit_to_code[the_char] = (A_BIT | G_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'h': punit_to_code[the_char] = (A_BIT | C_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'v': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          case 'n': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT | T_BIT);
            punit_to_code_for_data[the_char] = NO_BIT;
            break;
          default:
            punit_to_code[the_char] = 0;
            punit_to_code_for_data[the_char] = 0;
            break;
        }
        if (punit_to_code[the_char] & A_BIT) {
            punit_to_code[the_char] |= T_BIT << 4;
            punit_to_code_for_data[the_char] |= T_BIT << 4;
        }
        if (punit_to_code[the_char] & C_BIT) {
            punit_to_code[the_char] |= G_BIT << 4;
            punit_to_code_for_data[the_char] |= G_BIT << 4;
        }
        if (punit_to_code[the_char] & G_BIT) {
            punit_to_code[the_char] |= C_BIT << 4;
            punit_to_code_for_data[the_char] |= C_BIT << 4;
        }
        if (punit_to_code[the_char] & T_BIT) {
            punit_to_code[the_char] |= A_BIT << 4;
            punit_to_code_for_data[the_char] |= A_BIT << 4;
        }
    }

    for (the_char=0; the_char < 256; the_char++)
    {
        switch (the_char & 15)
        {
          case A_BIT:
            code_to_punit[the_char] = 'A';
            break;

          case C_BIT:
            code_to_punit[the_char] = 'C';
            break;

          case G_BIT:
            code_to_punit[the_char] = 'G';
            break;

          case T_BIT:
            code_to_punit[the_char] = 'T';
            break;

          case A_BIT + C_BIT:
            code_to_punit[the_char] = 'M';
            break;

          case A_BIT + G_BIT:
            code_to_punit[the_char] = 'R';
            break;

          case A_BIT + T_BIT:
            code_to_punit[the_char] = 'W';
            break;

          case C_BIT + G_BIT:
            code_to_punit[the_char] = 'S';
            break;

          case C_BIT + T_BIT:
            code_to_punit[the_char] = 'Y';
            break;

          case G_BIT + T_BIT:
            code_to_punit[the_char] = 'K'; 
            break;

          case C_BIT + G_BIT + T_BIT:
            code_to_punit[the_char] = 'B';
            break;

          case A_BIT + G_BIT + T_BIT:
            code_to_punit[the_char] = 'D';
            break;

          case A_BIT + C_BIT +T_BIT:
            code_to_punit[the_char] = 'H';
            break;

          case A_BIT + C_BIT + G_BIT:
            code_to_punit[the_char] = 'V';
            break;

          case A_BIT + C_BIT + G_BIT +T_BIT:
            code_to_punit[the_char] = 'N';
            break;
        }
    }
    conv_table_init = 1;
    return(0);
}

