#include "common.hpp"

using Self = PhysicalDevice;

Self::Features Self::features() const {
  Features out;
  vkGetPhysicalDeviceFeatures(handle, &out);
  return out;
}
Self::Properties Self::properties() const {
  Properties out;
  vkGetPhysicalDeviceProperties(handle, &out);
  return out;      
}

std::vector<QueueFamily> Self::get_queue_families() {
  auto tmp = unchecked_enumerate<VkQueueFamilyProperties>(
    vkGetPhysicalDeviceQueueFamilyProperties, handle
  );

  std::vector<QueueFamily> out { tmp.size() };
  for(u32 i = 0; i < tmp.size(); i++) {
    out[i] = QueueFamily {
      .parent = this,
      .index = i,
      .properties = tmp[i]
    };
  }
  return out;
}
