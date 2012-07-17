#ifndef FREELING_SPLITTER_H
#define FREELING_SPLITTER_H

#include <node.h>
#include <vector>
#include <string>

#include "freeling.h"
#include "freeling/morfo/util.h"
#include "v8-convert.hpp"
#include "helper.h"

class FreeLingSplitter : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);

private:
  FreeLingSplitter(const std::string);
  ~FreeLingSplitter();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Tokenize(const v8::Arguments& args);
  static v8::Handle<v8::Value> Split(const v8::Arguments& args);
  
  splitter *sp;
  tokenizer *tk;
  std::string path;
};

#endif