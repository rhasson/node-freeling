#ifndef FREELING_DEPENDENCY_PARSER_H
#define FREELING_DEPENDENCY_PARSER_H

#include <node.h>

class FreeLingDepParser : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);

private:
  FreeLingDepParser();
  ~FreeLingDepParser();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Parse(const v8::Arguments& args);
}

#endif