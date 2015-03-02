

#ifndef REPEAT_H
#define REPEAT_H

#include "punit.h"

class repeat: public punit {
  punit* ref;
  int complement;
  int ins;
  int del;
  int mis;
  int flex;
}

#endif
