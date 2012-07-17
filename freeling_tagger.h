#ifndef FREELING_TAGGER_H
#define FREELING_TAGGER_H

#include <node.h>

class FreeLingTagger : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);

private:
  FreeLingTagger();
  ~FreeLingTagger();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Tag(const v8::Arguments& args);
}

#endif