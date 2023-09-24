#include "piping_csum.h"
#include "checksumer_blake2b.h"
#include "checksumer_xxhash.h"

using namespace std;

namespace archi{

std::unique_ptr<Checksumer> make_csumer_for(const Checksum &csum)
{
	if (holds_alternative<Xx_hash>(csum))
		return make_unique<Checksumer_xxhash>();
	if (holds_alternative<Blake2b_hash>(csum))
		return make_unique<Checksumer_blake2b>();
	ASSERT(false);
	std::unreachable();
}

void Pipe_through_csumer::csumer(std::unique_ptr<Checksumer> &&cs)
{
	csumer_ = move(cs);
}

void Pipe_through_csumer::csumer_for(const Checksum &csum)
{
	csumer_ = make_csumer_for(csum);
}

Pipe_csum_out::Pipe_csum_out(const Checksum set_for)
{
	csumer_for(set_for);
}

Pipe_csum_out::Pipe_csum_out(std::unique_ptr<Checksumer> &&cs)
{
	csumer(move(cs));
}
void Pipe_csum_out::pump(u8 *from, u64 size)
{
	csumer()->update(from, size);
	pump_next(from, size);
}

Pipe_csum_in::Pipe_csum_in(const Checksum set_for)
{
	csumer_for(set_for);
}

Pipe_csum_in::Pipe_csum_in(std::unique_ptr<Checksumer> &&cs)
{
	csumer(move(cs));
}

Source::Pump_result Pipe_csum_in::pump(u8 *to, u64 size)
{
	auto res = pump_next(to, size);
	csumer()->update(to, res.pumped_size);
	return res;
}





}
