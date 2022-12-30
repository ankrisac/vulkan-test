#include "instance.hpp"


void vk_try(const VkResult res) {
  if (res != VK_SUCCESS) {
    std::cout << "Vulkan Error : " << res << std::endl;
  }
}


std::strong_ordering operator<=>(const Version& lhs, const Version& rhs) {
  // variant needs to be compared first, so we can't use the default impl

  auto cmps = { 
    lhs.variant <=> rhs.variant,
    lhs.major   <=> rhs.major,
    lhs.minor   <=> rhs.minor,
    lhs.patch   <=> rhs.patch 
  };

  for (auto cmp : cmps) {
    if(cmp != 0) return cmp;
  }
  return std::strong_ordering::equal;
};

std::ostream& operator<<(std::ostream& out, const Version& ver) {
  out << ver.major << "." << ver.minor << "." << ver.patch;

  if(ver.variant != 0) {
    out << " variant(" << ver.variant << ")";
  }
  return out;
}




template<typename T, typename Enumerator, typename... Args>
std::vector<T> enumerate(Enumerator fn, Args... args) {
  u32 count = 0;
  fn(args..., &count, nullptr);
  
  std::vector<T> vec { count };
  fn(args..., &count, vec.data());

  return vec;
}

Instance::Instance(const Desc& desc) {
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,

    .pApplicationName = "VulkanTest",
    .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0),

    .pEngineName = "sgfx",
    .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 0),
    .apiVersion = VK_API_VERSION_1_2
  };

  VkInstanceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info, 

    .enabledLayerCount = static_cast<u32>(desc.layers.size()),
    .ppEnabledLayerNames = desc.layers.data(),

    .enabledExtensionCount = static_cast<u32>(desc.extensions.size()),
    .ppEnabledExtensionNames = desc.extensions.data()
  };

  vk_try(vkCreateInstance(&info, nullptr, &handle));
}

Instance::~Instance() {
  vkDestroyInstance(handle, nullptr);
}