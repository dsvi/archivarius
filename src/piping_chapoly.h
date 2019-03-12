#pragma once
#include "piping.h"

class Chapoly{
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
	void set_key_word(std::string_view kw);
	static constexpr
	size_t key_size(){return sizeof(key_);}
	static constexpr
	size_t iv_size(){ return sizeof(iv_); }
private:
	u8 iv_[24];
	u8 key_[32];
};

class Pipe_chapoly_in: public Pipe_in{
public:
	virtual
	Pump_result pump(u8 *to, u64 size) override;
private:
	//std::vector<ui64> buffer_;
};



class Pipe_chapoly_out: public Pipe_out{
public:
	Pipe_chapoly_out();
	Pipe_chapoly_out(Chapoly &p);
	void set_params(Chapoly &p);
	virtual
	void pump(u8 *to, u64 size) override;
private:
	//std::vector<ui64> buffer_;
};


