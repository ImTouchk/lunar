#pragma once
#include <vulkan/vulkan.hpp>

namespace Render::Vk
{
	vk::Instance& getInstance();
	vk::Device& getDevice();
	vk::Queue& getGraphicsQueue();
	vk::Queue& getPresentQueue();
}
