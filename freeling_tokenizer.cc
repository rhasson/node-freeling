#define FREELING_TOKENIZER

#include <node.h>
#include <v8.h>
#include <assert.h>

#include "freeling_tokenizer.h"


static v8::Persistent<v8::String> callback_sym;
static v8::Persistent<v8::String> onerror_sym;

FreeLingTokenizer::FreeLingTokenizer(const std::string path) {
	util::init_locale(L"default");
	std::string file = "tokenizer.dat";
	tk = new tokenizer(util::string2wstring(path+file));
};
FreeLingTokenizer::~FreeLingTokenizer() {
	delete tk;
};

//create the constructor template function
static v8::Persistent<v8::FunctionTemplate> const_tpl;
void FreeLingTokenizer::Init(v8::Handle<v8::Object> target) {
	v8::HandleScope scope;

	//using the functionTemplate, create a function FreeLingTokenizer
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(FreeLingTokenizer::New);
	const_tpl = v8::Persistent<v8::FunctionTemplate>::New(tpl);
	const_tpl->InstanceTemplate()->SetInternalFieldCount(1);
	const_tpl->SetClassName(v8::String::NewSymbol("Tokenizer"));

	//create prototype chain for above function
	NODE_SET_PROTOTYPE_METHOD(const_tpl, "tokenize", FreeLingTokenizer::Tokenize);
	
	target->Set(v8::String::NewSymbol("Tokenizer"), const_tpl->GetFunction());

	callback_sym = NODE_PSYMBOL("callback");
	onerror_sym = NODE_PSYMBOL("onerror");
	NODE_DEFINE_CONSTANT(target, TOKENIZER);
}

/*
* FreeLingTokenizer class New operator as exposed to Javascript
* Arguments: path to to FreeLing tokenizer rules directory
* Returns: a V8 value(object), an instance of the FreeLingTokenizer object
*/
v8::Handle<v8::Value> FreeLingTokenizer::New(const v8::Arguments& args) {
	v8::HandleScope scope;

	std::string dir = "";
	if (args[0]->IsUndefined()) {
		dir = "/usr/local/share/freeling/en/";
	} else {
		/*v8::String::Utf8Value s(args[0]);
		dir = std::string(*s, s.length());*/
		dir = cvv8::CastFromJS<std::string>(args[0]->ToString());
	}
	
	//create an instance of the FreeLingTokenizer class
	FreeLingTokenizer* obj = new FreeLingTokenizer(dir);
	
	
	obj->path = dir;
	obj->Wrap(args.Holder());

	return args.This();
}

/*
* Write buffer
* write(flush, in_buff, in_off, in_len, out_buff, out_off, out_len)
* flush: bool
* in_buff, out_buff: v8::Object
* in_off, out_off, in_len, out_len: v8::Uint32Value
*/
v8::Handle<v8::Value> FreeLingTokenizer::Write(const v8::Arguments& args) {
	HandleScope scope;

	assert(args.length() == 7);

	FreeLingTokenizer* obj = node::ObjectWrap::Unwrap<FreeLingTokenizer>(args.Holder());
	//check if init completed before writing is allowed
	//assert(obj->init_done_ && "write before init");

	assert(!obj->write_in_progress_ && "write already in progress");
	obj->write_in_progress_ = true;

	unsigned int flush = args[0]->Uint32Value();
	std::wstring *in;
	std::wstring *out;
	size_t in_off, in_len, out_off, out_len;

	if (args[1]->IsNull()) {
		//just flush
		std::string nada('');
		in = nada;
		in_len = 0;
		in_off = 0;
	} else {
		assert(node::Buffer::HasInstance(args[1]));  //input buffer
		v8::Local<v8::Object> in_buf;
		in_buf = args[1]->ToObject();
		in_off = args[2]->Uint32Value();
		in_len = args[3]->Uint32Value();

		assert(in_off + in_len <= node::Buffer::Length(in_buf));  //if offset+length is larger than buffer size, throw error
		in = reinterpret_cast<std::wstring *>(node::Buffer::Data(in_buf) + in_off);
	}

	assert(node::Buffer::HasInstance(args[4]));  //output buffer
	v8::Local<v8::Object> out_buf;
	out_off = args[5]->Uint32Value();
	out_len = args[6]->Uint32Value();
	assert(out_off + out_len <= node::Buffer::Length(out_buf));
	//out = reinterpret_cast<std::string *>(node::Buffer::Data(out_buf) + out_off);

	//build up the UV work request
	uv_work_t* work_req = &(obj->work_req_);

	stream_ *strm = new stream_();
	strm.avail_in = in_len;
	strm.in = in;
	strm.avail_out = out_len;
	//strm->out = out;
	obj.strm_ = strm;
	obj->flush_ = flush;

	obj->chunk_size_ = out_len;

	uv_queue_work(uv_default_loop(),
					work_req,
					FreeLingTokenizer::Process,
					FreeLingTokenizer::After);
	obj->Ref();

	return obj->handle_;
}

/*
* Do work inside LibUV worker thread
* No V8 available inside the thread pool
*/
void FreeLingTokenizer::Process(uv_work_t* work_req) {
	FreeLingTokenizer* obj = container_of(work_req, FreeLingTokenizer, work_req_);
	
	unsigned long offs = 0;
	std::list<word> lw;
	std::wstring t = util::string2wstring(obj->strm_->in);
	obj->tk->tokenize(t, offs, lw);
	words = Helper::GetWordsArray(lw);
}

voice FreeLingTokenizer::After(uv_work_t* work_req) {
	HandleScope scope;
}

/*
* Tokenize function
* Arguments: text string to be tokenized
* Returns: array of words (tokens)
*/
v8::Handle<v8::Value> FreeLingTokenizer::Tokenize(const v8::Arguments& args) {
	v8::HandleScope scope;

	FreeLingTokenizer* obj = node::ObjectWrap::Unwrap<FreeLingTokenizer>(args.Holder());
	v8::Handle<v8::Array> words;
	unsigned long offs = 0;

	if (args.Length() > 0) {
		std::string text = cvv8::CastFromJS<std::string>(args[0]->ToString());

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
