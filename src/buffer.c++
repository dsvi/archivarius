#include "buffer.h"
#include "exception.h"
#include "globals.h"

namespace archi{


Buffer::Buffer()
{

}

void Buffer::resize(size_t new_size)
{
	if (new_size > max_size_){
		buff_.reset(new u8[new_size]);
		max_size_ = new_size;
	}
	size_ = new_size;
}


}
