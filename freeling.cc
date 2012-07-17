#define FREELING_BINDING
#include <node.h>
#include <v8.h>
#include "freeling_tokenizer.h"
#include "freeling_splitter.h"

extern "C" {
	static void InitAll(v8::Handle<v8::Object> target) {
		FreeLingTokenizer::Init(target);
		//FreeLingTagger::Init(target);
		FreeLingSplitter::Init(target);
		//FreeLingDepParser::Init(target);
		//FreeLingChartParser::Init(target);
	}

	NODE_MODULE(freeling, InitAll)
}