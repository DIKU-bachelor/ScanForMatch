#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Punit.h"
using namespace std;

int conv_table_init = 1;
char punit_to_code[256];
char punit_to_code_for_data[256];
char code_to_punit[256];

bool known_char_i[16] = {0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};
char known_char_index[16] =
                 {-1,0,1,-1,2,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1};


#define A_BIT 0x01              /* char bitfield */
#define C_BIT 0x02              /* char bitfield */
#define G_BIT 0x04              /* char bitfield */
#define T_BIT 0x08              /* char bitfield */
#define NO_BIT 0x00

#define max_var_size 10000

void pprint(char* text, int len) {
  for (int i = 0; i < len; i++) {
    cout << code_to_punit[text[i]];
  }
  cout << "\n";
}

Punit::Punit(char* data_s, char* data_e, int data_l, char* c, int typ){
    data_start = data_s;
    data_end = data_e;
    data_len = data_l;
    mlen = 0;
    strech = 0;
    code = c;
    type = typ;
}

int Punit::get_score() {
  cout << "Punit get_flex called, should not happen...\n";
  return 0;
}

int Punit::get_min_len() {
  cout << "Punit get_min_len called, should not happen...\n";
  return 0;
}

int Punit::get_max_len() {
  cout << "Punit get_max_len called, should not happen...\n";
  return 0;
}

void Punit::reset(void) {}

ret_t* Punit::search(ret_t* start) {
  cout << "Punit search called, should not happen...\n";
}

char Punit::known_char(char C) {
    return (known_char_i[(unsigned char)C]);
}

/* checks if a data base corresponds to a pattern base */
bool Punit::matches(char C1, char C2) {
     return (known_char((C1) & 15) && ((((C1) & 15) & ((C2) & 15)) == 
            (C1 & 15)));
}



/* exact constructer used by the parser */
Exact::Exact(char* data_s, char* data_e, int data_len, int le, char* c, 
             int i, int d, int m, int f, int type) : Punit(data_s, data_e, data_len, c, type){
    len = le;
    ins = i;
    del = d;
    mis = m;
    flex = f;
    c_ins = ins;
    c_del = del;
    c_mis = mis;
    c_flex = flex;
    stack = (stackent*)malloc(sizeof(stackent)*1000);
    match_stack = (stackent*)malloc(sizeof(stackent)*1000);
    for(i = 0; i < 1000; i++){
      match_lens[i] = 0;
    }
    found_matches = 0;
    match_list_len = -1;
}
/* FUNCTION TO MISMATCHES INSERTIONS DELETIONS (kan udkommenteres hvis den ikke
compiler*/
void Exact::stack_next(stackent* st,int nxtE, int N, 
                       char* p1, char* data, int one_len,
                       int two_len) {
  st[nxtE].p1=p1; 
  st[nxtE].p2=data; 
  st[nxtE].n1=one_len;
  st[nxtE].n2=two_len;
  st[nxtE].mis=c_mis;
  st[nxtE].ins=c_ins; 
  st[nxtE].del=c_del; 
  st[nxtent++].next_choice=N;
  //printf("stack_next nxtent = %i \n", nxtent);
}

void Exact::pop(stackent* st, int nxtent, 
         char** pattern, char** data, int* p_len, int* d_len, 
         int* p_mis, int* p_ins, int* p_del,
         int* ins_nxt_p, int* del_nxt_p){
  *pattern = st[nxtent].p1;
  *data = st[nxtent].p2;
  *p_len = st[nxtent].n1;
  *d_len = st[nxtent].n2;
  *p_mis = st[nxtent].mis;
  *p_ins = st[nxtent].ins;
  *p_del = st[nxtent].del;

  //handle where to go next
  if (stack[nxtent].next_choice == 1) {
    if (*p_del){
      st[nxtent].next_choice = 2;
    }
    //printf("del next \n"); OLOLO
    *ins_nxt_p = 1;
  }else{
    *del_nxt_p = 1;
  }
}


/* Match saved in the match_stack */
void Exact::match(){
  //printf("match p2 = %p, c_mis = %i, c_ins = %i, c_del = %i \n", p2, c_mis, c_ins, c_del);
  match_stack[found_matches].p2=p2;
  match_stack[found_matches].mis=c_mis;
  match_stack[found_matches].ins=c_ins; 
  match_stack[found_matches++].del=c_del;
}


/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */ 

ret_t* Exact::search(ret_t* retu){
  //printf("new exact, retu->startp = %p, retu->len = %i\n", retu->startp, retu->len);
  if(retu->startp != NULL) {
    //printf("new exact \n");
    if (retu->opt) {
      opti = 1;
      retu->opt = 0;
    }
    if (retu->bst) {
      best = 1;
      retu->bst = 0;
    }
    prev = retu->startp;
    len_s = len;
    code_s = code;
    //variable declaration for loose matching pattern
    run_len = retu->len;
    run_len_s = run_len;
    one_len = len;
    two_len = data_end-prev;
    del_nxt = 0;
    ins_nxt = 0;
    mlen = 0;
    p1 = code;
    p2 = prev;
    flex = ins + del + mis;
    mlen_start = retu->startp;

  // Else we are at backtracking
  } else {
//    cout << "EXACT prev: " << prev - data_start << "\n";
    //printf("backtrack exact flex = %i\n", flex);
    mlen = 0;
  }
  int first = 0;
  if (run_len_s == data_len) {
    first = 1;
  }

  // If no mismatches, insertions or deletions allowed
  if(flex == 0){
    if (one_len > two_len) {
      retu->startp = NULL;
      return retu;
    }
    int i;
    int c = 0;

    char* prev_s = prev;
    if (retu->startp == NULL && run_len >= 0) {
      //printf("hello\n");
      prev++;
    }
    if (quick_ref) {
      //printf("  problem?! retu->startp = %p, retu->len = %i, comp = %i \n", retu->startp, retu->len, comp);
      if (comp) {
        // If this punit is a complementary and previous punit is the variable being set
        p3 = code + len + (run_len_s - run_len) - 1;
        while (prev_s <= prev_s + run_len-- && prev + len + (prev - prev_s) < data_end) {
//          cout << "EXACT PREV : " << prev << "\n";
          if (matches(*prev,(((*p3++) >> 4) & 15))){
            p2 = prev+1;p3 = p3 - 2;
            c = prev - prev_s;
            for (i = (len + c) - 1; i && matches(*p2,(((*p3) >> 4) & 15)); i--,p3--,p2++)
              ;
            if (!i){
//              cout << "MAUATCH\n";
              c = prev - prev_s;
              len += c;
              mlen = len;
              break;
            }
          }
          prev++;
        }

      // If this punit is a non-complementary and previous punit is the variable being set
      } else {
        //printf("looking?!\n");
        while (prev_s <= prev_s + run_len-- && prev + len + (prev - prev_s) < data_end) {
          if (matches(*prev,*code)){
            p2 = prev+1; p3 = code+1;
            c = prev - prev_s;
//            cout << c << "\n";
            for (i = (len + c) - 1; i && matches(*p2,*p3); i--,p3++,p2++)
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
    // If this Punit is complementary and the previous Punit was not the variable being set
      if (comp) {
//        cout << "hej\n";
        p3 = code + len - 1;
        while (0 <= run_len-- && data_end - (prev + len) > 0) {          
          if (matches(*prev,((*(p3) >> 4) & 15))){
            p2 = prev+1; p4 = p3 - 1;
            for (i = len - 1; i && matches(*p2,(((*p4) >> 4) & 15)); i--,p4--,p2++)
              ;
            if (!i){
              c = prev - prev_s;
              mlen = len;
              break;
            }
          }
//          cout << "loumi\n";
          prev++;
        }
      } else {
      // If this Punit is not complementary and the previous Puni was not the variable being set
        //printf("prev = %p, prevc = %c, codec = %c \n", prev, *prev, *code);
        while (prev_s <= prev_s + run_len-- && prev + len < data_end) {
          if (matches(*prev,*code)){
            p2 = prev+1;p3 = code+1;
            if (*p3 == '\0') {
              c = prev - prev_s;
              mlen = len;
              break;
            }
            for (i=len-1; i && matches(*p2,*p3); i--,p3++,p2++)
              ;
            if (!i){
/*              for (int k = 0; k < len; k++) {
                printf("%c", code_to_punit[prev[k]]);
              }
              cout << " match " << run_len << "\n"; */
              c = prev - prev_s;
              mlen = len;
              break;
            }
          }
          prev++;
        }
      }
    }
    retu->quick_ref = 0;
    retu->len = 0;
    //printf("len = %i, mlen = %i \n", len, mlen);
    // If we have a match
    if(len == mlen){
//      cout << len << "\n";
      if ((first != 1) && (opti != 1)) {
        mlen += c;
        mlen = (prev + len) - mlen_start;
      }
      retu->startp = (prev + len);
      /* Both the variable and the reference is c longer, 
         so mlen is incremented twice with c */
      if (retu->quick_ref) {
        mlen += c;
      }
      return retu;
    } else {
      retu->startp = NULL;
      return retu;
    }
  /* Loose match in the case where there are:
     Insertions, deletions and mismatches */
  } else {
    int i;
    int c = 0;
    ret_t* new_retu = new ret_t;

    char* prev_s = prev;
    /*if (retu->startp == NULL && run_len >= 0) {
      prev++;
      //run_len++;
    }*/
    if (quick_ref) {
      if (comp) {
        // If this punit is a complementary and previous punit is the variable being set
        while (0 <= run_len && 
               prev + len + (prev - prev_s) < data_end) {
          code = code_s + run_len;
          len = len_s + prev - prev_s;
          new_retu = loose_match(retu, new_retu);
          if(new_retu->startp){
            mlen = mlen + prev - prev_s;
            return new_retu;
          }
          retu->startp = ++prev; p2++; run_len--;
        }

      // If this punit is a non-complementary and previous punit is the variable being set
      } else {
        while (prev_s <= prev_s + run_len-- && 
               prev + len + (prev - prev_s) < data_end) {
          len = len_s + prev - prev_s;
          new_retu = loose_match(retu, new_retu);
          if(new_retu->startp){
            mlen = mlen + prev - prev_s;
            return new_retu;
          }
          retu->startp = ++prev; p2++; //run_len--;
        }
      }
    } else {
//      cout << "MID prev: " << prev << "\n";
//      cout << "MID runlen: " << run_len << "\n";
//      cout << match_list_len << "\n";
      if (match_list_len > 0) {
        new_retu = loose_match(retu, new_retu);
        return new_retu;
      }
      // If this Punit is not complementary and the previous Punit was not the variable being set
      while (prev_s <= prev_s + run_len && prev + len < data_end) {
//        cout << "loose start pos: " << prev << "\n";
        new_retu = loose_match(retu, new_retu);
        if(new_retu->startp){
//          cout << "loose matched\n";
          if ((! first) && (! opti)) {
            mlen = mlen + prev - prev_s;
          }
          return new_retu;
        }
        retu->startp = ++prev; p2++; run_len--;
      }
    }
  }
  retu->startp = NULL;
  retu->len = 0;
  return retu;
}



//Loose match, in the case of insertions, deletions and mismatches
//new
ret_t* Exact::loose_match (ret_t* retu, ret_t* new_retu){
 // printf("loosematch, prev = %p, prevc = %c, retu->startp = %p\n", prev, *prev, retu->startp);
  //Backtracking
  if(retu->startp == NULL){
//    printf("backtrack match_list_len = %i \n", match_list_len);
    if(match_list_len >= 0){
//      cout << "next len " <<  match_list[match_list_len] << "\n";
      new_retu->startp = (char*)(match_list[match_list_len--]);
      new_retu->len = 0;
      mlen = new_retu->startp - prev;
      return new_retu;
    } else {
      new_retu->startp = NULL;
      new_retu->len = 0;
      return new_retu;
    }
  }
  //New position
  if (retu->startp != NULL) {
//    cout << "LOOSE: ikke backtrack..." << retu->startp << "\n";
    //Initiation at next position
    one_len = len;
    two_len = data_end-prev;
    del_nxt = 0;
    ins_nxt = 0;
    mlen = 0;
    p1 = code;
    p2 = prev;
    flex = ins + del + mis;
    c_mis = mis;
    c_ins = ins;
    c_del = del;
    nxtent = 0;
    found_matches = 0;
    memset(match_lens, 0, sizeof(match_lens));
    match_list_len = 0;

    //Search
    //Special-case for ins=del=0 
    if ((c_ins == 0) && (c_del == 0)){
      if (one_len > two_len)
      {
        new_retu->startp = NULL;
        new_retu->len = 0;
        return new_retu;
      }
      int i;
      for (i=one_len; i >= 1; i--){
        if (!known_char((*p2)&15) ||
           (!matches(*p2,*p1) && (--c_mis < 0))){
          new_retu->startp = NULL;
          new_retu->len = 0;
          return new_retu;
        } else {
            p2++; p1++;
        }
      }
      new_retu->startp = (char*)(p2);
      new_retu->len = 0;
      mlen  = (int) (p2 - prev);
      return new_retu;
    }
    // With insertions and deletions
    while (1){
    //printf("ins = %i, del = %i, mis = %i, one_len = %i, p2 = %p, p2c = %c, c = %p cc = %c\n", c_ins, c_del, c_mis, one_len, p2, *p2, p1, *p1);
      //Match
      if (!del_nxt && !ins_nxt 
          && (two_len && one_len >= 1 && known_char((*p2)&15) 
          && matches(*p2,*p1))){
        p2++; p1++; two_len--;
        if (!(--one_len)){
          match();
        }
      }
      // Mismatch
      else if (!del_nxt && !ins_nxt &&(c_mis && (one_len >= 1) && (two_len >= 1))){
        if (c_ins){
          stack_next(stack, nxtent, 1, p1, p2, one_len, two_len);
        }
        else if (c_del){
          stack_next(stack, nxtent, 2, p1, p2, one_len, two_len);
        }
        c_mis--; p1++; p2++; one_len--; two_len--;
        if (! one_len)
        {
          match();
        }
      }
      //Insertion
      else if (!del_nxt && (ins_nxt  || (c_ins) && (one_len >= 1))){
        if (!ins_nxt && (c_del) && (two_len >= 1)){
          stack_next(stack, nxtent, 2, p1, p2, one_len, two_len);
        }
        ins_nxt = 0;
        c_ins--; p1++; one_len--;
        if (! one_len){
          match();
        }
      }
      //Deletion
      else if (del_nxt || ((c_del) && (two_len >= 1))){
        del_nxt = 0;
        c_del--; p2++; two_len--;
        if (! one_len){
          match();
        }
      }
      else if (nxtent--){
        pop(stack, nxtent, 
         &p1, &p2, &one_len, &two_len, 
         &c_mis, &c_ins, &c_del,
         &ins_nxt, &del_nxt);
      } else{
        break; 
      }
    }
    int n;

//    printf("matches? = %i\n", found_matches);
// match_list is list of pointers that points to different new starting positions
//    cout << "LOOSE: foundmatches: " << found_matches << "\n";
    if(found_matches-- > 0){
//      printf("m1: %s\n", match_stack[found_matches].p2);
//      printf("m1i: %i\n", match_stack[found_matches].del);
      //printf("found_matches = %i \n", found_matches);
      for(found_matches; found_matches >= 0; found_matches--){
        //Move match len with insertions
        for(n = match_stack[found_matches].ins; 
            n && match_stack[found_matches].p2 - prev - n > 0; 
            n--){
          if(!match_lens[match_stack[found_matches].p2 - prev - n]){
            match_lens[match_stack[found_matches].p2 - prev - n] = 1;
            match_list[match_list_len] = match_stack[found_matches].p2 - n;
            //printf("ins match = %p \n", match_list[match_list_len]);
            match_list_len++;
          }
        }
        //Move match len with deletions
        for(n = match_stack[found_matches].del; n; n--){
          if(!match_lens[match_stack[found_matches].p2 - prev + n]){
            match_lens[match_stack[found_matches].p2 - prev + n] = 1;
            match_list[match_list_len] = match_stack[found_matches].p2 + n;
            //printf("del match = %p \n", match_list[match_list_len]);
            match_list_len++;
          }
        }
        // save the original match length
        if(!match_lens[match_stack[found_matches].p2 - prev]){
          //printf("orig match = %p \n", match_stack[found_matches].p2);
          match_lens[match_stack[found_matches].p2 - prev] = 1;
          match_list[match_list_len] = match_stack[found_matches].p2;
          //printf("ori match = %p, prev = %p \n", match_list[match_list_len], prev);
          match_list_len++;
        }
      }
      match_list_len--;
      /*for(n = match_list_len; n >= 0; n--){
        //printf("match_list match = %p \n", match_list[n]);
      }*/
      new_retu->startp = match_list[match_list_len--];
      new_retu->len = 0;
      mlen = new_retu->startp - prev;
//      cout << "LOOSE mlen: " << mlen << "\n";
      //printf("new_retu->startp = %p, match_list_len = %i\n", new_retu->startp, match_list_len);
//      cout << "LOOSE: next pos: " <<  new_retu->startp << "\n";
      return new_retu;
    }
  }
  new_retu->startp = NULL;
  new_retu->len = 0;
  return new_retu;
}

void Exact::reset(void) {
  mlen = 0;
  c_ins = ins;
  c_del = del;
  c_mis = mis;
  c_flex = flex;
}

int Exact::get_score() {
  return len - (mis + ins + del);
}

int Exact::get_min_len() {
  return len - ins;
}

int Exact::get_max_len() {
  return len + del;
}

/* Range constructer used by the parser */
Range::Range(char* data_s, char* data_e, int data_l, int le, char* c, 
             int w, int type) : Punit(data_s, data_e, 0, c, type){
    len = le;
    width = w;
    data_len = data_l;
    code = new char[1000];
}



/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */
ret_t* Range::search(ret_t* retu){
  int first = 0;
  if (retu->len == data_len) {
    first = 1;
  }
  // The next punit will know that this is a variable being set
  if(retu->startp == NULL) {
    // it can't backtrack any more
//    cout << "RANGE backtrack inc: " << inc_width << "\n"; 
    if(inc_width == 0){
      //printf("range retu = %p \n", retu->startp);
      return retu;
    // backtracking one forward
    } else {
      inc_width--;
      strech++;
      if ((! first) && (! opti)) {
        mlen++;
      }
      prev++;
      if (prev + len < data_end) {
        retu->len = width;
        retu->startp = prev + len;
/*        orig_code = prev;
        strncpy(code, orig_code, len + width); */
      }
      //printf("range 2 retu = %p \n", retu->startp);
      return retu;
    }
  }
  mlen = len;
  strech = 0;
  prev = retu->startp;
  if (retu->opt) {
    opti = 1;
    retu->opt = 0;
  }
/*  orig_code = retu->startp;
  strncpy(code, orig_code, len + width); */
//  cout << "RANGE prev: " << prev << "\n";
  inc_width = retu->len;
  if(retu->startp + len < data_end){
    retu->startp = (retu->startp + len);
    retu->len = width;
    mlen = len;
    return retu;
  } else {
    retu->startp = NULL;
    retu->len = 0;
    return retu;
  }
}

void Range::reset(void) {
  mlen = 0;
}

int Range::get_score() {
  return 0;
}

int Range::get_min_len() {
  return len;
}

int Range::get_max_len() {
  return len + width;
}

Reference::Reference(char* data_s, char* data_e, int data_len, int comp, Range* var_p, 
              Punit* nxt_p, int first, int mis, int ins, int del, int flex, int type)
           : Exact(data_s, data_e, data_len, 0, NULL, mis, ins, del, flex, type) {
  variable = var_p;
  next_Punit = nxt_p;
  complement = comp;
  first_ref = first;
  cCode = new char[1000];
}

//Reference search
ret_t* Reference::search(ret_t* retu) {
  if (retu->startp) {
    code = variable->prev;
    prev = retu->startp;
    //printf("next_Punit->prev = %p, variable->prev = %p \n", next_Punit->prev, variable->prev);
    prev = retu->startp;
    len = next_Punit->prev - variable->prev;
    /*is_amb = 0;
    for (int i = 0; i < len + variable->width; i++) {
      if ((code[i] & 15) & ((code[i] >> 4) & 15)) {
        is_amb = 1;
        break;
      }
    } */
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
      //printf("loose match ref ins = %i, del = %i\n", ins, del);
      comp = complement;
      counter1 = len - 1;
      if (quick_ref) {
        counter1 += variable->width;
      }
      /*if (ins) {
        //printf("extra del used\n");
        counter1 += ins;
      }*/
      counter2 = 0;
      while(counter1 >= 0 && code + counter1 <= data_end){
        cCode[counter2++] = ((code[counter1--] >> 4) & 15);
      }
      //printf("1\n");
      code = cCode; 
    }
  }
  return Exact::search(retu);
}

int Reference::get_score() {
  return 0;
}

int Reference::get_min_len() {
  return variable->len - ins;
}

int Reference::get_max_len() {
  return variable->len + variable->width + del;
}

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

