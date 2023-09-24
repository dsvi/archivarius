#include "encryption_params.h"
#include "exception.h"

namespace archi{


void Encryption_params::randomize()
{
	auto &rng = Botan::system_rng();
	rng.randomize(iv_, sizeof (iv_));
	rng.randomize(key_, sizeof (key_));
}

void Encryption_params::randomize_iv()
{
	auto &rng = Botan::system_rng();
	rng.randomize(iv_, sizeof (iv_));
}


void Encryption_params::set_password(std::string_view kw)
{
	if (kw.empty())
		throw Exception("Requires password");
	Botan::Blake2b csum(256);
	csum.update((u8*)kw.data(), kw.size());
	ASSERT(csum.output_length() == sizeof (key_));
	csum.final(key_);
}


}
