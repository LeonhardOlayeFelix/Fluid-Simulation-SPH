#include "VertexBufferLayout.h"

VertexBufferLayout::VertexBufferLayout(std::initializer_list<unsigned int> nums)
{
	for (unsigned int n : nums) {
		push<float>(n);
	}
}