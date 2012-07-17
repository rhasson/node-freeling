#ifndef HELPER_H
#define HELPER_H

#include <vector>
#include <list>
#include <string>
#include <v8.h>

#include "freeling.h"
#include "freeling/morfo/util.h"
#include "v8-convert.hpp"

class Helper {
public:
  static v8::Handle<v8::Array> GetSentencesArray(const std::list<sentence>& ls);
  static v8::Handle<v8::Array> GetWordsArray(const std::list<word>& ls);
};

#endif