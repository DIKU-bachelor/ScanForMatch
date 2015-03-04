#include "punit.h"

void punit::set_mlen(int l){
    mlen = len;
}

void punit::set_code(char* c){
    code = c;
}

void punit::set_prev(char* p){
    prev = p;
}

int punit::get_mlen(void){
    return mlen;
}

char* punit::get_code(void){
    return code;
}

char* punit::get_prev(void){
    return prev;
}
