#pragma once
#include "precomp.h"

namespace archi{


class Encryption_params{
public:
	u8* key(){
		return key_;
	};
	u8* iv(){
		return iv_;
	}

	template<class T>
	void key(T &v){
		ASSERT( (uint) distance(begin(v), end(v)) <= sizeof(key_) );
		auto i = begin(v);
		for (auto &a: key_){
			a = *i++;
		}
	}

	template<class T>
	void iv(T &v){
		ASSERT( (uint) distance(begin(v), end(v)) <= sizeof(iv_) );
		auto i = begin(v);
		for (auto &a: iv_){
			a = *i++;
		}
	}

	/// set random key and iv
	void randomize();
	/// increment iv by 1
	void inc_iv();
	/// set key from arbitrary string
	void set_password(std::string_view kw);
	static constexpr
	size_t key_size(){return sizeof(key_);}
	static constexpr
	size_t iv_size(){ return sizeof(iv_); }
private:
	u8 iv_[24];
	u8 key_[32];
};


}
