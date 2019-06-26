#pragma once
#include "stream.h"

namespace archi{

/**
 * @brief pumps from `in` up to `to` into `out`, using the provided buffer.
 * @param num_already_pumped current position in `in`. Should be less than `to`. The position is updated to current state
 */
void pump(Stream_in &in, u64 to, Stream_out *out, std::string_view fname, Buffer &tmp, u64 &num_already_pumped );

}
