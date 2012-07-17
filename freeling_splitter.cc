#define FREELING_SPLITTER

#include <node.h>
#include <v8.h>
#include <node_buffer.h>

#include "freeling_splitter.h"

FreeLingSplitter::FreeLingSplitter(const std::string path) {
	util::init_locale(L"default");
	std::string t_file = "tokenizer.dat";
	std::string s_file = "splitter.dat";
	tk = new tokenizer(util::string2wstring(path+t_file));
	sp = new splitter(util::string2wstring(path+s_file));
	//Splitter tk(path);
	//v_tk.push_back(tk);
};
FreeLingSplitter::~FreeLingSplitter() {
	delete tk;
	delete sp;
};

//create the constructor template function
static v8::Persistent<v8::FunctionTemplate> const_tpl;
void FreeLingSplitter::Init(v8::Handle<v8::Object> target) {
	v8::HandleScope scope;

	//using the functionTemplate, create a function FreeLingSplitter
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(FreeLingSplitter::New);
	const_tpl = v8::Persistent<v8::FunctionTemplate>::New(tpl);
	const_tpl->InstanceTemplate()->SetInternalFieldCount(1);
	const_tpl->SetClassName(v8::String::NewSymbol("Splitter"));

	//create prototype chain for above function
	NODE_SET_PROTOTYPE_METHOD(const_tpl, "tokenize", FreeLingSplitter::Tokenize);
	NODE_SET_PROTOTYPE_METHOD(const_tpl, "split", FreeLingSplitter::Split);
	
	target->Set(v8::String::NewSymbol("Splitter"), const_tpl->GetFunction());
}

/*
* FreeLingSplitter class New operator as exposed to Javascript
* Arguments: path to to FreeLing Splitter rules directory
* Returns: a V8 value(object), an instance of the FreeLingSplitter object
*/
v8::Handle<v8::Value> FreeLingSplitter::New(const v8::Arguments& args) {
	v8::HandleScope scope;

	std::string dir = "";
	if (args[0]->IsUndefined()) {
		dir = "/usr/local/share/freeling/en/";
	} else {
		dir = cvv8::CastFromJS<std::string>(args[0]->ToString());
	}
	
	//create an instance of the FreeLingSplitter class
	FreeLingSplitter* obj = new FreeLingSplitter(dir);
		
	obj->path = dir;
	obj->Wrap(args.Holder());

	return args.This();
}

/*
* Tokenize function
* Arguments: text string to be tokenized
* Returns: array of words (tokens)
*/
v8::Handle<v8::Value> FreeLingSplitter::Tokenize(const v8::Arguments& args) {
	v8::HandleScope scope;

	FreeLingSplitter* obj = node::ObjectWrap::Unwrap<FreeLingSplitter>(args.Holder());
	v8::Handle<v8::Array> words;
	unsigned long offs = 0;

	if (args.Length() > 0) {
		if (v8::Buffer::HasInstance(args[0])){
			v8::Local<v8::Object> buf_obj = args[0]->ToObject();
			std::string text = string(v8::Buffer::Data(buf_obj), v8::Buffer::Length(buf_obj));
		} else {
			std::string text = cvv8::CastFromJS<std::string>(args[0]->ToString());
		}
		//handle arguments and process data
		if (text.length() > 0) {
			std::list<word> lw;
			std::wstring t = util::string2wstring(text);
			obj->tk->tokenize(t, offs, lw);
			words = Helper::GetWordsArray(lw);
		}
	} else {
		return ThrowException(v8::Exception::TypeError(v8::String::New("Missing first argument: the text to be tokenized")));
	}
	return scope.Close(words);
}

/*
* Split function
* Arguments: list of tokenized word forms (returned from Tokenizer)
* Returns: array of sentences
*/
v8::Handle<v8::Value> FreeLingSplitter::Split(const v8::Arguments& args) {
	v8::HandleScope scope;

	FreeLingSplitter* obj = node::ObjectWrap::Unwrap<FreeLingSplitter>(args.Holder());
	v8::Handle<v8::Array> sentences;
	unsigned long offs = 0;

	if (args.Length() > 0) {
		std::string text = cvv8::CastFromJS<std::string>(args[0]->ToString());
		bool flush = args[1]->IsBoolean() ? cvv8::CastFromJS<bool>(args[1]->ToBoolean()) : true;

		if (text.length() > 0) {
			std::list<word> lw;
			std::list<sentence> ls;
			std::wstring t = util::string2wstring(text);
			obj->tk->tokenize(t, offs, lw);
			obj->sp->split(lw, flush, ls);
			sentences = Helper::GetSentencesArray(ls);
			lw.clear();
			ls.clear();
		}
	} else {
		return ThrowException(v8::Exception::TypeError(v8::String::New("Missing first argument: text to be split into sentences")));
	}
	return scope.Close(sentences);
}
