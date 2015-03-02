#include "exact.h"

exact::exact(int l, int i, int d, int m, int f, char* in_code, char* in_prev){
    len = l;
    ins = i;
    del = d;
    mis = m;
    flex = f;
    set_code(in_code);
    set_prev(in_prev);
    set_mlen(0);
}



/* If start is NULL, the previous search failed, and this search starts at prev.
   If start is not NULL, prev in this punit is set to start and is initialized */
char* exact::search(char* start){
    
    return start;
}
