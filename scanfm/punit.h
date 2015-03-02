

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
}
