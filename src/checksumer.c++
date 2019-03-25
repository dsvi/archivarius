#include "checksumer.h"

using namespace std;

Checksumer::Checksumer()
{
	set_for(Xx_hash());
}

void Checksumer::set_for(const Checksum &csum)
{
	if (holds_alternative<Xx_hash>(csum)){
		auto &c = cs_pipe_holder_.emplace<Pipe_xxhash_out>();
		cs_ = &c;
		cs_pipe_ = &c;
	}
	if (holds_alternative<Blake2b_hash>(csum)){
		auto &c = cs_pipe_holder_.emplace<Pipe_blake2b_out>();
		cs_ = &c;
		cs_pipe_ = &c;
	}
}

