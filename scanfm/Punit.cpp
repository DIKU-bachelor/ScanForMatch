#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "Punit.h"
using namespace std;

int initialized = 1;
char punit_to_code[256];
char code_to_punit[256];

/* for loose fitet patterns (inserts, deletions, mismatches) */
struct stackent {
  unsigned char *p1,*p2;
  int n1,n2;
  int mis,ins,del;
  int next_choice;
};

bool known_char_i[16] = {0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};
char known_char_index[16] =
                 {-1,0,1,-1,2,-1,-1,-1,3,-1,-1,-1,-1,-1,-1,-1};


#define A_BIT 0x01              /* char bitfield */
#define C_BIT 0x02              /* char bitfield */
#define G_BIT 0x04              /* char bitfield */
#define T_BIT 0x08              /* char bitfield */


Punit::Punit(char* c){
    mlen = 0;
    code = c;
}

void Punit::reset(void) {}

char* Punit::search(char* start) {
  cout << "OASIDJOSIDJ";
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
Exact::Exact(int le, char* c, 
             int i, int d, int m, int f) : Punit(c){
    len = le;
    ins = i;
    del = d;
    mis = m;
    flex = f;
    c_ins = ins;
    c_del = del;
    c_mis = mis;
    c_flex = flex;
}
/* FUNCTION TO MISMATCHES INSERTIONS DELETIONS (kan udkommenteres hvis den ikke
compiler*/
/*void Exact::stack_next(struct stackent st,int nxtE, int N, 
                       char* p1, char* d1, int one_len,
                       int two_len) {
  st[nxtE].p1=p1; 
  st[nxtE].p2=d1; 
  st[nxtE].n1=one_len;
  st[nxtE].n2=two_len;
  st[nxtE].mis=c_mis;
  st[nxtE].ins=c_ins; 
  st[nxtE].del=c_del; 
  st[nxtE++].next_choice=N;
}*/
/* FUNCTION TO MISMATCHES INSERTIONS DELETIONS
#define Pop                                                 \
            one_data = stack[nxtent].p1;                    \
            two_data = stack[nxtent].p2;                    \
            one_len = stack[nxtent].n1;                     \
            two_len = stack[nxtent].n2;                     \
            max_mis = stack[nxtent].mis;                    \
            max_ins = stack[nxtent].ins;                    \
            max_del = stack[nxtent].del;                    \
            if (stack[nxtent].next_choice == 1)             \
            {                                               \
                if (max_del)                                \
                    stack[nxtent].next_choice = 2;          \
                goto ins_char;                              \
            }                                               \
            else                                            \
                goto del_char;
*/
/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */ 
char* Exact::search(char* start){
  if(start != NULL) {
    prev = start;
  }
/* MISMATCHES INSERTIONS DELETIONS CODE
  if(flex == 0){
    int i;
    char* p1 = code;
    char* p2 = prev;
    for( i = len; i && matches(*p2, *p1); i--, p1++, p2++){
      mlen++;
    }
    if(len == mlen){
      return prev + mlen + 1;
    }
  } else{
// loose_match from scan_for_match    
//unsigned char *one_data = pattern, *two_data = data;
//int one_len, two_len;
//int *match_range;

    int i, nxtent;
    stackent* stack = (stackent*)malloc(sizeof(stackent)*100);



    // special-case for ins=del=0 
    if ((c_ins == 0) && (c_del == 0))
    {
        if (one_len > two_len)
        {
            return NULL;
        }
        for (i=one_len; i >= 1; i--)
        {
            if (!known_char((*two_data)&15) ||
               (matches(*two_data,*one_data) && (--c_mis < 0)))
                return NULL;
            else
            {
                two_data++; one_data++;
            }
        }
	return (two_data-prev)+1;
    }

    nxtent=0;
    while (two_len || nxtent)
    {
        if (two_len && one_len && KnownChar((*two_data)&15) &&
	    ExMatches(rule_set,*two_data,*one_data))
        {
            two_data++; one_data++; two_len--;
            if (!(--one_len))
                return (two_data - prev)+1;
        }
	else if (c_mis && (one_len >= 1) && (two_len >= 1))
	{
	    if (c_ins)
	    {
        stack_next(stack, nxtent, 1, one_data, two_data, one_len, two_len);
	    }
	    else if (c_del)
	    {
        stack_next(stack, nxtent, 2, one_data, two_data, one_len, two_len);
	    }
	    c_mis--; one_data++; two_data++; one_len--; two_len--;
	    if (! one_len)
	    {
	      return (two_data - prev)+1;
	    }
	}
	else if ((c_ins) && (one_len >= 1))
	{
	    if ((c_del) && (two_len >= 1))
	    {
        stack_next(stack, nxtent, 2, one_data, two_data, one_len, two_len);
	    }
	  ins_char:
	    c_ins--; one_data++; one_len--;
	    if (! one_len)
	        return (two_data - prev)+1;
	}
	else if ((c_del) && (two_len >= 1))
	{
	  del_char:
	    c_del--; two_data++; two_len--;
	    if (! one_len)
	      return (two_data - prev)+1;
	}
	else if (nxtent--)
	{
	  Pop;
	}
	else
	  return NULL;
    }
    return NULL;
}
*/
  int i;
  char* p1 = code;
  char* p2 = prev;
  for(i = len; i && matches(*start, *p1); i--, p1++, p2++){
    mlen++;
  }
  return NULL;
}

void Exact::reset(void) {
  mlen = 0;
  c_ins = ins;
  c_del = del;
  c_mis = mis;
  c_flex = flex;
}


/* Range constructer used by the parser */
Range::Range(int le, char* c, 
             int w) : Punit(c){
    len = le;
    width = w;
}



/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */
char* Range::search(char* start){
  if(start != NULL) {
    prev = start;
  }
  char* p2 = prev;
  if(mlen < len) {
    mlen = len;
    return (p2+mlen);
  } else if(mlen < len + width - 1){
    mlen++;
    return (p2 + mlen);
  } 
  return NULL;
}

void Range::reset(void) {
  mlen = 0;
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

