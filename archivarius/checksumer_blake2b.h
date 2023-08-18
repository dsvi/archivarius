#pragma once

#include "checksumer.h"

namespace archi{

class Checksumer_blake2b : public Checksumer
{
public:
	Checksum checksum() override;
	void reset() override;
	void update(u8 *from, u64 size) override;
private:
	Botan::Blake2b state_{512};
};

}
