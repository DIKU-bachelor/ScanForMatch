#ifndef PUNIT_H
#define PUNIT_H

#include <list>

extern char punit_to_code[256];
extern char code_to_punit[256];

typedef struct ret {
  char* startp;
  int len;
  int match_len;
  char* var;
  char* pcode;
} ret_t;

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
    char* data_start;
    char* data_end;
    int data_len;
    int run_len;
    int mlen;
    char* code;
    char* prev;
    Punit(char* data_s, char* data_e, int data_l, char* c);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual ret_t* search(ret_t* retu);
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
    Exact(char* data_s, char* data_e, int data_len, int le, char* c, int i, int d, int m, int f);
    void reset(void);
    ret_t* search(ret_t* retu);
};


/* punit range inherites from punit, is used to jump in the data e.g
*  6..8 jumps from 6 to 8 charecters
*  in the data */
class Range: public Punit {
  public:
    int len;
    int width;
    Range(char* data_s, char* data_e, int le, char* c, int w);
    void reset(void);
    ret_t* search(ret_t* retu);
};

class Complementary: public Exact{
  public:
    char* cCode; //complementary code
    bool newCode; //true if the code has not yet been made complementary
    Complementary(char* data_s, char* data_e, int data_len, int le, char* c, 
                  int i, int d, int m, int f);
    void reset(void);
    ret_t* search(ret_t* retu);
};

class Variable: public Exact {
  public:
    int len;
    int width;
    char* var_name;
    Variable(char* data_s, char* data_e, int data_len, char* name, int le, char* c, int w);
    ret_t* search(ret_t* retu);
};

class Reference: public Exact {
  public:
    int complement;
    Reference(char* data_s, char* data_e, int data_len, int comp, Punit* var, Punit* next_p, 
      int mis, int ins, int del, int flex);
    ret_t* search(ret_t* retu);
};

#endif
