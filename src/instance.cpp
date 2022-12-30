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

  std::vector<T> data { count };
  fn(args..., &count, data.data());
  return data;
}

template<typename T, typename S, typename GetName>
bool check_support(
  std::ostream&         log,
  const std::vector<T>& available, 
  const std::vector<S>& required,
  GetName               view
) {
  bool any_missing = false;

  for (auto name : required) {
    bool found = false;

    for(auto avail : available) {
      if(std::strcmp(name, view(avail)) == 0) {
        found = true;
        break;
      }   
    }

    if(!found) any_missing = true;

    std::cout
      << (found ? "[X]" : "[ ]")  
      << " " << name << std::endl; 
  }
  return any_missing;
}

const InstanceDesc& InstanceDesc::check_support(std::ostream& log) const {
  using Layer = VkLayerProperties;
  std::cout << "Checking Layer support" << std::endl;
  ::check_support(
    log,
    enumerate<Layer>(vkEnumerateInstanceLayerProperties),
    layers.vec(),
    [](Layer layer) { return layer.layerName; } 
  );

  using Extension = VkExtensionProperties;
  std::cout << "Checking Extension support" << std::endl;
  ::check_support(
    log,
    enumerate<Extension>(vkEnumerateInstanceExtensionProperties, nullptr),
    extensions.vec(),
    [](Extension ext) { return ext.extensionName; }
  );

  return *this;
}
Instance InstanceDesc::build() const {
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

    .enabledLayerCount = static_cast<u32>(layers.size()),
    .ppEnabledLayerNames = layers.data(),

    .enabledExtensionCount = static_cast<u32>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data()
  };

  VkInstance handle;
  vk_try(vkCreateInstance(&info, nullptr, &handle));
  return Instance { handle };
}
