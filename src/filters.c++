#include "filters.h"

Filtrator_in::Filtrator_in(Filters_in &f)
{
	set_filters(f);
}

Pipe_in &Filtrator_in::apply(Pipe_in &p)
{
	Pipe_in *prev = &p;
	if (cmp_pipe_in_){
		*prev << *cmp_pipe_in_;
		prev = &*cmp_pipe_in_;
	}
	if (enc_pipe_in_){
		*prev << *enc_pipe_in_;
		prev = &*enc_pipe_in_;
	}
	return *prev;
}

void Filtrator_in::set_filters(Filters_in &f)
{
	if (f.cmp_in)
		compression(f.cmp_in.value());
	if (f.enc_in)
		encryption(f.enc_in.value());
}

void Filtrator_in::compression(Zstd_in &zin)
{
	cmp_pipe_in_.emplace();
}

void Filtrator_in::encryption(Chapoly &ein)
{
	//TODO:
}


Pipe_out &Filtrator_out::apply(Pipe_out &p)
{
	Pipe_out *prev = &p;
	if (cmp_pipe_out_){
		*prev >> *cmp_pipe_out_;
		prev = &*cmp_pipe_out_;
	}
	if (enc_pipe_out_){
		*prev >> *enc_pipe_out_;
		prev = &*enc_pipe_out_;
	}
	return *prev;
}

void Filtrator_out::set_filters(Filters_out &f)
{
	if (f.cmp_out)
		compression(f.cmp_out.value());
	if (f.enc_out)
		encryption(f.enc_out.value());
}

Filters_in Filtrator_out::get_filters()
{
	Filters_in ret;
	if (enc_out_)
		ret.enc_in = enc_out_;
	if (cmp_out_)
		ret.cmp_in.emplace();
	return ret;
}

void Filtrator_out::flush_der_kompressor()
{
	if (cmp_pipe_out_)
		cmp_pipe_out_->flush();
}

