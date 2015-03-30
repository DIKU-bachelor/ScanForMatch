#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "Punit.h"
using namespace std;

int initialized = 1;
char punit_to_code[256];
char code_to_punit[256];


bool known_char_i[16] = {0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};
char known_char_index[16] =
                 {-1,0,1,-1,2,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1};


#define A_BIT 0x01              /* char bitfield */
#define C_BIT 0x02              /* char bitfield */
#define G_BIT 0x04              /* char bitfield */
#define T_BIT 0x08              /* char bitfield */

#define max_var_size 10000

struct var {
  char* name;
  char* code;
};

list<var*> vtable;

Punit::Punit(char* data_s, char* data_e, int data_l, char* c){
    data_end = data_e;
    data_len = data_l;
    mlen = 0;
    code = c;
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
             int i, int d, int m, int f) : Punit(data_s, data_e, data_len, c){
    len = le;
    ins = i;
    del = d;
    mis = m;
    flex = f;
    c_ins = ins;
    c_del = del;
    c_mis = mis;
    c_flex = flex;
    stack = (stackent*)malloc(sizeof(stackent)*100);
}
/* FUNCTION TO MISMATCHES INSERTIONS DELETIONS (kan udkommenteres hvis den ikke
compiler*/
void Exact::stack_next(stackent* st,int* nxtE, int N, 
                       char* p1, char* data, int one_len,
                       int two_len) {
  st[*nxtE].p1=p1; 
  st[*nxtE].p2=data; 
  st[*nxtE].n1=one_len;
  st[*nxtE].n2=two_len;
  st[*nxtE].mis=c_mis;
  st[*nxtE].ins=c_ins; 
  st[*nxtE].del=c_del; 
  st[*nxtE++].next_choice=N;
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
      *ins_nxt_p = 1;
    }
    else{
      *del_nxt_p = 1;
    }
  }
}
/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */ 

char* appendChar(char* arr, char a) {
  size_t l = strlen(arr);
  char* r = new char[l + 2];
  strcpy(r, arr);
  r[l] = a;
  r[l + 1] = '\0';
  return r;
}

ret_t* Exact::search(ret_t* retu){
  if(retu->startp != NULL) {
    prev = retu->startp;

    //variable declaration for loose matching pattern
    run_len = retu->len;
    one_len = len;
    two_len = data_end-prev;
    del_nxt = 0;
    ins_nxt = 0;
    mlen = 0;
    p1 = code;
    p2 = prev;
  // Else we are at backtracking
  } else {
    retu->match_len -= mlen;
    mlen = 0;
    // If we're also at the first punit, we increment data with 1
  }

  // If no mismatches, insertions or deletions allowed
  if(flex == 0){
    if (one_len > two_len) {
      retu->startp = NULL;
      return retu;
    }
    int i;
    int c = 0;

   // Is len used for allowing scan throught possibly whole file
    if (run_len == data_len) {
      if (retu->startp == NULL) {
        prev++;
      }
      while (prev <= data_end) {
        if (matches(*prev,*code)){
          p2 = prev+1;char* p3 = code+1;
          for (i=len-1; i && matches(*p2,*p3); i--,p3++,p2++)
            ;
          if (!i){
            mlen = len;
            break;
          }
        }
        prev++;
      }

    // If len is used for flexibility due to previous Range punit
    } else if (strcmp(retu->var, "\0") == 0) {
      if (retu->startp == NULL && run_len >= 0) {
        prev++;
        run_len--;
      }
      char* prev_s = prev;
      while (prev_s <= prev_s + run_len--) {
        if (matches(*prev,*code)){
          p2 = prev+1;char* p3 = code+1;
          for (i=len-1; i && matches(*p2,*p3); i--,p3++,p2++)
            ;
          if (!i){
            c = prev - prev_s;
            mlen = len;
            break;
          }
        }
        prev++;
      }

    // If the previous range unit was a variable being set, we need to save the letters
    // for future reference
    } else {
      if (retu->startp == NULL && run_len >= 0) {
        prev++;
        run_len--;
      }
      char* prev_s = prev;
      while (prev_s <= prev_s + run_len--) {
        if (matches(*prev,*code)){
          p2 = prev+1;char* p3 = code+1;
          for (i=len-1; i && matches(*p2,*p3); i--,p3++,p2++)
            ;
          if (!i){
            c = prev - prev_s;
            mlen = len;
            break;
          }
        }
        retu->pcode = appendChar(retu->pcode, *prev);
        prev++;
      }

    }
    retu->len = 0;
    if(len == mlen){
      if (strcmp(retu->var, "\0") == 0) {
        cout << "huhu\n";
        for (list<var*>::iterator it = vtable.begin(); it != vtable.end(); it++) {
          cout << retu->var << "\n";
          cout << (*it)->name << "\n";
          if (strcmp(retu->var, (*it)->name) == 0) {
            (*it)->code = retu->pcode;
            cout << (*it)->name << "\n";
            cout << (*it)->code << "\n";
            break;
          }
        }
      }
      mlen += c;
      retu->startp = (prev + len);
      retu->match_len += mlen;
      return retu;
    } else {
      retu->startp = NULL;
      return retu;
    }
  // OBS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Tilføjelse til MID koden: retu->match_len skal tælles op
  } else {
    if (retu->startp != NULL) {
      c_mis = mis;
      c_ins = ins;
      c_del = del;
      lml.clear();
    } 
    // special-case for ins=del=0 
    if ((c_ins == 0) && (c_del == 0)){
      if (one_len > two_len)
      {
        retu->startp = NULL;
        retu->len = 0;
        return retu;
      }
      int i;
      for (i=one_len; i >= 1; i--){
        if (!known_char((*p2)&15) ||
           (!matches(*p2,*p1) && (--c_mis < 0))){
          retu->startp = NULL;
          retu->len = 0;
          return retu;
        }
        else
        {
            p2++; p1++;
        }
      }
      retu->startp = (char*)(p2)+1;
      retu->len = 0;
      return retu;
    }

    nxtent=0;
    while (two_len || nxtent){
      if (!del_nxt && !ins_nxt && (two_len && one_len && known_char((*p2)&15) &&
	      matches(*p2,*p1))){
        p2++; p1++; two_len--;
        if (!(--one_len)){
          int succes = 1;
          for(list<char*>::iterator list_iter = lml.begin(); 
              list_iter != lml.end(); list_iter++){
            if(*list_iter == p1){
              succes = 0;
            }
          }
          if(succes){
            lml.push_back(p1);

            retu->startp = (char*)(p2)+1;
            retu->len = 0;
            return retu;
          }
        }
      }
      else if (!del_nxt && !ins_nxt &&(c_mis && (one_len >= 1) && (two_len >= 1))){
	      if (c_ins){
          stack_next(stack, &nxtent, 1, p1, p2, one_len, two_len);
	      }
	      else if (c_del){
          stack_next(stack, &nxtent, 2, p1, p2, one_len, two_len);
	      }
	      c_mis--; p2++; p2++; one_len--; two_len--;
	      if (! one_len)
	      {
          int succes = 1;
          for(list<char*>::iterator list_iter = lml.begin(); 
              list_iter != lml.end(); list_iter++){
            if(*list_iter == p1){
              succes = 0;
            }
          }
          if(succes){
            lml.push_back(p1);

            retu->startp = (char*)(p2)+1;
            retu->len = 0;
            return retu;
          }
	      }
	    }
	    else if (!del_nxt && (ins_nxt  || (c_ins) && (one_len >= 1))){
	      if (!ins_nxt && (c_del) && (two_len >= 1)){
          stack_next(stack, &nxtent, 2, p1, p2, one_len, two_len);
	      }
        ins_nxt = 0;
	      c_ins--; p1++; one_len--;
	      if (! one_len){
          int succes = 1;
          for(list<char*>::iterator list_iter = lml.begin(); 
              list_iter != lml.end(); list_iter++){
            if(*list_iter == p1){
              succes = 0;
            }
          }
          if(succes){
            lml.push_back(p1);
            retu->startp = (char*)(p2)+1;
            retu->len = 0;
            return retu;
          }
	      }
	    }
	    else if (del_nxt || ((c_del) && (two_len >= 1))){
        del_nxt = 0;
        c_del--; p2++; two_len--;
        if (! one_len){
          int succes = 1;
          for(list<char*>::iterator list_iter = lml.begin(); 
              list_iter != lml.end(); list_iter++){
            if(*list_iter == p1){
              succes = 0;
            }
          }
          if(succes){
            lml.push_back(p1);
              retu->startp = (char*)(p2)+1;
              retu->len = 0;
              return retu;
          }
        }
	    }
	    else if (nxtent--){
	      pop(stack, nxtent, 
         &p1, &p2, &one_len, &two_len, 
         &c_mis, &c_ins, &c_del,
         &ins_nxt, &del_nxt);
	    }
	    else{
        retu->startp = NULL;
        retu->len = 0;
        return retu;
      }
    }
  }
  retu->startp = NULL;
  retu->len = 0;
  return retu;
}

void Exact::reset(void) {
  mlen = 0;
  c_ins = ins;
  c_del = del;
  c_mis = mis;
  c_flex = flex;
}


/* Range constructer used by the parser */
Range::Range(char* data_s, char* data_e, int le, char* c, 
             int w) : Punit(data_s, data_e, 0, c){
    len = le;
    width = w;
}



/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */
ret_t* Range::search(ret_t* retu){
  if(retu->startp == NULL) {
    retu->match_len -= mlen;
    return retu;
  }
//  cout << "RANGE\n"; 
  retu->startp = (retu->startp + len);
  retu->len = width;
  mlen = len;
  retu->match_len += len;
  return retu;
}

void Range::reset(void) {
  mlen = 0;
}

Complementary::Complementary(char* data_s, char* data_e, int data_len, int le, char* c, 
             int i, int d, int m, int f) : Exact(data_s, data_e, data_len, le, c, 
             i, d, m, f){
  cCode = (char*)malloc(1000*sizeof(char));
  newCode = true;
}

ret_t* Complementary::search(ret_t* sp){
  if(newCode){
    int i = len-1;
    int s = 0;
    while(i >= 0){
      switch(code[i] & 15){
        case A_BIT:
          cCode[s] = punit_to_code['t'];
          break;
        case T_BIT:
          cCode[s] = punit_to_code['a'];
          break;
        case C_BIT:
          cCode[s] = punit_to_code['g'];
          break;
        case G_BIT:
          cCode[s] = punit_to_code['c'];
          break;
        default:
          break;
      }
      i--;
      s++;
    }
    strncpy(code, cCode, 1000);
    printf("code is now: %s \n", cCode);
    newCode = false;
  }
  return Exact::search(sp);
}

void Complementary::reset(void){

}

Variable::Variable(char* data_s, char* data_e, int data_len, char* name, int le, char* c, int w)
          : Exact(data_s, data_e, data_len, le, c, 0, 0, 0, 0) {
  len = le;
  width = w;
  mlen = 0;
  code = new char[max_var_size];
  var* v = new var();
  v->name = name;
  var_name = name;
  vtable.push_back(v);
}

ret_t* Variable::search(ret_t* retu) {
  if (retu->startp == NULL) {
    retu->match_len -= mlen;
    return retu;
  }
  code = new char[max_var_size];
  for (int i = 0; i < len; i++) {
    code[i] = retu->startp[i];
  }
  cout << "VARIABLE search: " << code << "\n";
  retu->startp = (retu->startp + len);
  retu->len = width;
  mlen = len;
  retu->match_len += len;
  retu->var = var_name;
  retu->pcode = code;
  return retu;
}

Reference::Reference(char* data_s, char* data_e, int data_len, char* name, int comp, 
           int mis, int ins, int del, int flex) 
           : Exact(data_s, data_e, data_len, 0, NULL, mis, ins, del, flex) {
  var_name = name;
  complement = comp;
}

ret_t* Reference::search(ret_t* retu) {
  cout << "REFERENCE search\n";
  for (list<var*>::iterator it = vtable.begin(); it != vtable.end(); it++) {
    if (strcmp(var_name, (*it)->name) == 0) {
      cout << "HEJ\n";
      code = (*it)->code;
      cout << "DAV\n";
      break;
    }
  }
  cout << "REFERENCE fandt var med kode: " << code << "\n";
}

int build_conversion_tables()
{
    int the_char;

    for (the_char=0; the_char < 256; the_char++) {
        switch(tolower(the_char)) {
          case 'a': punit_to_code[the_char] = A_BIT; break;
          case 'c': punit_to_code[the_char] = C_BIT; break;
          case 'g': punit_to_code[the_char] = G_BIT; break;
          case 't': punit_to_code[the_char] = T_BIT; break;
          case 'u': punit_to_code[the_char] = T_BIT; break;
          case 'm': punit_to_code[the_char] = (A_BIT | C_BIT); break;
          case 'r': punit_to_code[the_char] = (A_BIT | G_BIT); break;
          case 'w': punit_to_code[the_char] = (A_BIT | T_BIT); break;
          case 's': punit_to_code[the_char] = (C_BIT | G_BIT); break;
          case 'y': punit_to_code[the_char] = (C_BIT | T_BIT); break;
          case 'k': punit_to_code[the_char] = (G_BIT | T_BIT); break;
          case 'b': punit_to_code[the_char] = (C_BIT | G_BIT | T_BIT); break;
          case 'd': punit_to_code[the_char] = (A_BIT | G_BIT | T_BIT); break;
          case 'h': punit_to_code[the_char] = (A_BIT | C_BIT | T_BIT); break;
          case 'v': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT); break;
          case 'n': punit_to_code[the_char] = (A_BIT | C_BIT | G_BIT | T_BIT); break;
          default:
            punit_to_code[the_char] = 0;
            break;
        }
        if (punit_to_code[the_char] & A_BIT)
            punit_to_code[the_char] |= T_BIT << 4;
        if (punit_to_code[the_char] & C_BIT)
            punit_to_code[the_char] |= G_BIT << 4;
        if (punit_to_code[the_char] & G_BIT)
            punit_to_code[the_char] |= C_BIT << 4;
        if (punit_to_code[the_char] & T_BIT)
            punit_to_code[the_char] |= A_BIT << 4;
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
    initialized=1;
    return(0);
}

