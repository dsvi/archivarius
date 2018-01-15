#pragma once
#include "precomp.h"

class Buffer
{
public:
	Buffer();
	void resize(size_t new_size);
	size_t size(){
		return  size_;
	}
	ui8* raw(){
		return buff_.get();
	}
private:
	std::unique_ptr<ui8[]> buff_{};
	size_t size_;
	size_t max_size_{};
};




