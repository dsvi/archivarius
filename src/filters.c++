#include "filters.h"

Filtrator_in::Filtrator_in()
{

}

Filtrator_in::Filtrator_in(Filters_in &f)
{
	if (f.cmp_in)
		compression(f.cmp_in.value());
	ASSERT(!(f.enc_chapo_in and f.enc_chacha_in));

	if (f.enc_chapo_in)
		encryption(f.enc_chapo_in.value());
	if (f.enc_chacha_in)
		encryption(f.enc_chacha_in.value());
}

Pipe_in &Filtrator_in::apply(Pipe_in &p)
{
	Pipe_in *prev = &p;
	if (cmp_pipe_in_){
		*prev << *cmp_pipe_in_;
		prev = &*cmp_pipe_in_;
	}
	if (enc_pipe_chapo_in_){
		*prev << *enc_pipe_chapo_in_;
		prev = &*enc_pipe_chapo_in_;
	}
	if (enc_pipe_chacha_in_){
		*prev << *enc_pipe_chacha_in_;
		prev = &*enc_pipe_chacha_in_;
	}
	return *prev;
}

void Filtrator_in::compression(Zstd_in &zin)
{
	cmp_pipe_in_.emplace();
}

void Filtrator_in::encryption(Chapoly &ein)
{
	ASSERT(!enc_pipe_chacha_in_);
	enc_pipe_chapo_in_.emplace(ein);
}

void Filtrator_in::encryption(Chacha &ein)
{
	ASSERT(!enc_pipe_chapo_in_);
	enc_pipe_chacha_in_.emplace(ein);
}

Pipe_out &Filtrator_out::apply(Pipe_out &p)
{
	Pipe_out *prev = &p;
	if (cmp_pipe_out_){
		*prev >> *cmp_pipe_out_;
		prev = &*cmp_pipe_out_;
	}
	if (enc_pipe_chacha_out_){
		*prev >> *enc_pipe_chacha_out_;
		prev = &*enc_pipe_chacha_out_;
	}
	if (enc_pipe_chapo_out_){
		*prev >> *enc_pipe_chapo_out_;
		prev = &*enc_pipe_chapo_out_;
	}
	return *prev;
}

void Filtrator_out::compression(Zstd_out zout)
{
	cmp_out_.emplace(zout);
	cmp_pipe_out_.emplace(zout);
}

void Filtrator_out::encryption(Chapoly &eout)
{
	ASSERT(!enc_pipe_chacha_out_);
	enc_pipe_chapo_out_.emplace(eout);
	enc_chapo_out_.emplace(eout);
}

void Filtrator_out::encryption(Chacha &eout)
{
	ASSERT(!enc_pipe_chapo_out_);
	enc_pipe_chacha_out_.emplace(eout);
	enc_chacha_out_.emplace(eout);
}

void Filtrator_out::set_filters(Filters_out &f)
{
	// do not set twice
	ASSERT(!enc_pipe_chacha_out_ and !enc_pipe_chapo_out_ and !cmp_pipe_out_);
	if (f.cmp_out)
		compression(f.cmp_out.value());
	ASSERT(!(f.enc_chapo_out and f.enc_chacha_out));
	if (f.enc_chapo_out)
		encryption(f.enc_chapo_out.value());
	if (f.enc_chacha_out)
		encryption(f.enc_chacha_out.value());
}

Filters_in Filtrator_out::get_filters()
{
	Filters_in ret;
	if (enc_chapo_out_)
		ret.enc_chapo_in = *enc_chapo_out_;
	if (enc_chacha_out_)
		ret.enc_chacha_in = *enc_chacha_out_;
	if (cmp_out_)
		ret.cmp_in.emplace();
	return ret;
}

void Filtrator_out::flush_der_kompressor()
{
	if (cmp_pipe_out_)
		cmp_pipe_out_->flush();
}

