

class punit {
  int mlen;
  char* prev;
  public:
    const char* code;
   /* Resets the punit, e.g. range type punit mlen = min */
    virtual void initialize();
    /* If start is NULL, the previous search failed, and this search starts at prev.
       If start is not NULL, prev in this punit is set to start and is initialized */
    virtual char* search(char* start);
};

void punit::initialize(){}

char* punit::search(char* start) {
  return start;
}

class exact: public punit {
  public:
   int len;
   int ins;
   int del;
   int mis;
   int flex;
   exact(int, const char*, int, int, int, int);
   void initialize();
   char* search(char* start);
};

exact::exact(int length, const char* pat, int in, int de, int mi, int fle) {
  len = length;
  code = pat;
  ins = in;
  del = de;
  mis = mi;
  flex = fle;
}

void exact::initialize() {
  int x = 1;
}

char* exact::search(char* start) {
  return start;
}
