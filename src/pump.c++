#include "precomp.h"
#include "pump.h"
#include "exception.h"

namespace archi{


Pump::Pump()
{
	tmp.resize(128*1024);
}

void Pump::pump(Stream_in &in, u64 to, Stream_out *out, std::string_view fname, u64 &num_already_pumped )
{
	ASSERT(num_already_pumped <= to);
	while (num_already_pumped < to){
		auto num_left = to - num_already_pumped;
		auto to_pump = num_left > tmp.size() ? tmp.size() : num_left;
		auto ret = in.pump(tmp.raw(), to_pump);
		if (ret.eof && ret.pumped_size != to_pump)
			throw Exception( "Truncated content file {0}" )(fname);
		if (out)
			out->pump(tmp.raw(), to_pump);
		num_already_pumped += to_pump;
	}
	ASSERT(num_already_pumped == to);
}


}
