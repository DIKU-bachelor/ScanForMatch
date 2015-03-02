

class punit {
  int mlen;
  char* code;
  char* prev;
  public:
    /* Resets the punit, e.g. range type punit mlen = min */
    virtual void initialize(void);
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual char* search(char* start);
}
