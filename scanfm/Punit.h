#ifndef PUNIT_H
#define PUNIT_H

extern char punit_to_code[256];
extern char code_to_punit[256];

int build_conversion_tables(void);

class Punit {
  public:
    int mlen;
    char* code;
    char* prev;
    Punit(char* c);
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
  public:
    int len;
    int ins;
    int del;
    int mis;
    int flex;
    Exact(int le, char* c, int i, int d, int m, int f);
    void reset(void);
    char* search(char* start);
};
#endif
