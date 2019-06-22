#pragma once
#include "precomp.h"
#include "stream.h"
#include "piping_zstd.h"
#include "piping_xxhash.h"
#include "piping_chapoly.h"
#include "piping_chacha.h"

namespace archi{


struct Filters_in{
	std::optional<Zstd_in> cmp_in;
	std::optional<Chapoly> enc_chapo_in;
	std::optional<Chacha> enc_chacha_in;
	operator bool() const { return cmp_in or enc_chapo_in or enc_chacha_in; }
};

class Filtrator_in
{
public:
	Filtrator_in();
	explicit
	Filtrator_in(Filters_in &f);
	/// applies filters to p and returns the last of them
	Pipe_in &apply(Pipe_in &p);

	void compression(Zstd_in &zin);
	void encryption(Chapoly &ein);
	void encryption(Chacha &ein);
private:
	std::optional<Pipe_zstd_in> cmp_pipe_in_;
	std::optional<Pipe_chapoly_in> enc_pipe_chapo_in_;
	std::optional<Pipe_chacha_in> enc_pipe_chacha_in_;
};

inline
Pipe_in & operator << (Pipe_in &p, Filtrator_in &f){
	return f.apply(p);
}

struct Filters_out{
	std::optional<Zstd_out> cmp_out;
	std::optional<Chapoly> enc_chapo_out;
	std::optional<Chacha>  enc_chacha_out;
};


class Filtrator_out
{
public:
		/// applies filters to p and returns the last of them
	Pipe_out & apply(Pipe_out &p);

	void compression(Zstd_out zout);
	void encryption(Chapoly &eout);
	void encryption(Chacha &eout);
	void set_filters(Filters_out &f);
	Filters_in get_filters();

	void flush_der_kompressor();
private:
	std::optional<Pipe_zstd_out> cmp_pipe_out_;
	std::optional<Pipe_chapoly_out> enc_pipe_chapo_out_;
	std::optional<Pipe_chacha_out>  enc_pipe_chacha_out_;
	std::optional<Zstd_out> cmp_out_;
	std::optional<Chapoly> enc_chapo_out_;
	std::optional<Chacha>  enc_chacha_out_;
};

inline
Pipe_out & operator >> (Pipe_out &p, Filtrator_out &f){
	return f.apply(p);
}


}
