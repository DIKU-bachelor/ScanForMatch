#ifndef PUNIT_H
#define PUNIT_H

char punit_to_code[256];
char code_to_punit[256];


int build_conversion_tables(void);

class Punit {
  int mlen;
  char* code;
  char* prev;
  public:
    Punit(int l, char* c, char* p);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual char* search(char* start)=0;
    /* set the length of mlen */
    void set_mlen(int l);
    /* set code start pointer of pattern code */
    void set_code(char* c);
    /* set start point for the punit */
    void set_prev(char* p);

    /* get mlen */
    int get_mlen(void);
    /* get code pointer */
    char* get_code(void);
    /*get data location, prev */
    char* get_prev(void);

    char known_char(char C);

    /* Match 2 given bases with eachother using the converted bit type*/
    bool matches(char C1, char C2);
};




/* punit exact inherites from punit, is used to search for a litteral
*  in the data */
class Exact: public Punit {
  int len;
  int ins;
  int del;
  int mis;
  int flex;
  public:
    Exact(int l, char* c, char* p, int le, int i, int d, int m, int f);
    char* search(char* start);
};
#endif
