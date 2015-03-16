#ifndef PUNIT_H
#define PUNIT_H

#include <list>

extern char punit_to_code[256];
extern char code_to_punit[256];

/* for loose fitet patterns (inserts, deletions, mismatches) */
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
    char* data_end;
    int mlen;
    char* code;
    char* prev;
    Punit(char* data_e, char* c);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual char* search(char* start);
    virtual void reset(void);
    char known_char(char C);

    /* Match 2 given bases with eachother using the converted bit type*/
    bool matches(char C1, char C2);
};




/* punit exact inherites from punit, is used to search for a litteral
*  in the data */
class Exact: public Punit {
    char* p1;
    char* p2;
    int i, nxtent, one_len, two_len, del_nxt, ins_nxt;
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
    //last match lengths
    std::list<char*> lml;
    void stack_next(stackent* st,int* nxtE, int N, 
                       char* p1, char* d1, int one_len,
                       int two_len);
    void pop(stackent* st, int nxtent, 
                    char** pattern, char** data, int* p_len, int* d_len, 
                    int* p_mis, int* p_ins, int* p_del,
                    int* ins_nxt_p, int* del_nxt_p);
    Exact(char* data_e, int le, char* c, int i, int d, int m, int f);
    void reset(void);
    char* search(char* start);
};


/* punit range inherites from punit, is used to jump in the data e.g
*  6..8 jumps from 6 to 8 charecters
*  in the data */
class Range: public Punit {
  public:
    int len;
    int width;
    Range(char* data_e, int le, char* c, int w);
    void reset(void);
    char* search(char* start);
};

class Complementary: public Exact{
    public:
    char* cCode; //complementary code
    bool newCode; //true if the code has not yet been made complementary
    Complementary(char* data_e, int le, char* c, 
                  int i, int d, int m, int f);
    void reset(void);
    char* search(char* start);
};


#endif
