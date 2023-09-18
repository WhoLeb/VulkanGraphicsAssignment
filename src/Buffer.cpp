#include "Buffer.h"

namespace assignment
{
	Buffer::Buffer(
		Device& device,
		VkDeviceSize instanceSize,
		uint32_t instanceCount,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize minOffsetAlignment)
			:device(device),
			instanceSize(instanceSize),
			instanceCount(instanceCount),
			usageFlags(usageFlags),
			memoryPropertyFlags(memoryPropertyFlags)
	{
	}

}
