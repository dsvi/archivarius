#pragma once
#include "checksumer.h"

namespace archi {

class Checksumer_xxhash : public Checksumer{
public:
	Checksum checksum() override;
	void reset() override;
	void update(u8 *from, u64 size) override;
private:
	xxh::hash_state64_t state_{0};
};

}
