#define HELPER

#include <v8.h>

#include "helper.h"


/*
* Extracts word_form from list of tokenized words in a sentence
* Arguments: list of Word elements
* Returns: v8 Array with the word_form of each word in the tokenized list
*/

v8::Handle<v8::Array> Helper::GetWordsArray(const std::list<word>& ls) {
	v8::HandleScope scope;

	std::list<std::string> ary;
	for (std::list<word>::const_iterator i=ls.begin(); i!=ls.end(); i++) {
		std::wstring t(i->get_form());
		ary.push_back(util::wstring2string(t));
	}
	v8::Handle<v8::Value> a = cvv8::CastToJS(ary);
	v8::Handle<v8::Array> newV8Array = v8::Handle<v8::Array>::Cast(a);
	return newV8Array;
}

/*
* Extracts sentences from analyzed text
* Arguments: list of Sentence elements
* Returns: v8 Array with the sentences
*/

v8::Handle<v8::Array> Helper::GetSentencesArray(const std::list<sentence>& ls) {
	v8::HandleScope scope;

	std::wstring sent = L"";
	sentence::const_iterator w;
	std::list<std::string> ary;
	for (std::list<sentence>::const_iterator s=ls.begin(); s!=ls.end(); s++) {
		for (w=s->begin(); w!=s->end(); w++) {
			std::wstring t(w->get_form());
			sent = sent+t+L" ";
		}
		ary.push_back(util::wstring2string(sent));
		sent = L"";
	}

	v8::Handle<v8::Value> a = cvv8::CastToJS(ary);
	v8::Handle<v8::Array> newV8Array = v8::Handle<v8::Array>::Cast(a);
	return newV8Array;
}
