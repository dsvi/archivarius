#pragma once
#include "stream.h"

namespace archi{

/**
 * @brief pumps data from a Stream_in up to a byte position, optionally into a Stream_out.
 * Owns the temporary buffer it pumps through.
 */
class Pump
{
public:
	Pump();
	/**
	 * @brief pumps from `in` up to `to` into `out`, using the internal buffer.
	 * @param num_already_pumped current position in `in`. Should be less than `to`. The position is updated to current state
	 * @param out could be null
	 */
	void pump(Stream_in &in, u64 to, Stream_out *out, std::string_view fname, u64 &num_already_pumped);
private:
	Buffer tmp;
};

}
