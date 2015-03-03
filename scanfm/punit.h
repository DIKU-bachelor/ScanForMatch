char punit_to_code[256];
char code_to_punit[256];
int initialized;

int build_conversion_tables(void);

class punit {
  int mlen;
  char* code;
  char* prev;
  public:
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual char* search(char* start);
    /* set the length of mlen */
    void set_mlen(int len);
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

    bool known_char(char* C);

    /* Match 2 given bases with eachother using the converted bit type*/
    bool Matches(char* C1, char* C2);
}





/* punit exact inherites from punit, is used to search for a litteral
*  in the data */
class exact: public punit {
  int len;
  int ins;
  int del;
  int mis;
  int flex;
  public:
    exact(int l, int i, int d, int m, int f, char* in_code, char* in_prev);
}

