#pragma once
#include "precomp.h"

namespace archi{

class Buffer
{
public:
	Buffer();
	void resize(size_t new_size);
	size_t size(){
		return  size_;
	}
	u8* raw(){
		return buff_.get();
	}
private:
	std::unique_ptr<u8[]> buff_{};
	size_t size_{};
	size_t max_size_{};
};

}

