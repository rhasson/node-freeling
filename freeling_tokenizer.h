#ifndef FREELING_TOKENIZER_H
#define FREELING_TOKENIZER_H

#include <node.h>
#include <vector>
#include <string>

#include "freeling.h"
#include "freeling/morfo/util.h"
#include "v8-convert.hpp"
#include "helper.h"
#include "node_buffer.h"

class FreeLingTokenizer : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);

private:
  FreeLingTokenizer(const std::string);
  ~FreeLingTokenizer();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> Tokenize(const v8::Arguments& args);
  static v8::Handle<v8::Value> Write(const v8::Arguments& args);
  static void Process(uv_work_t *work_req);
  
  tokenizer *tk;
  std::string path;
  uv_work_t work_req_;
  bool write_in_progress_;
  stream_ *strm_;

  struct stream_ {
  	unsigned int avail_in;
  	std::wstring in;
  	unsigned int avail_out;
  	std::list<word> out;
  };
};

#endif