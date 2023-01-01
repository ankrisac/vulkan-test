#include "common.hpp"
#include <iostream>


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


DebugLog::DebugLog(VkInstance parent): parent(parent) 
{
  auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
    vkGetInstanceProcAddr(parent, "vkCreateDebugUtilsMessengerEXT")
  );
  create(parent, &create_info, nullptr, &handle);
}
DebugLog::~DebugLog() 
{
  if(handle != nullptr) {  
    auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(parent, "vkDestroyUtilsMessengerEXT")
    );
    destroy(parent, handle, nullptr);
  }
}
DebugLog::DebugLog(DebugLog&& other) {
  std::swap(parent, other.parent);
  std::swap(handle, other.handle);  
}

namespace Instance {
  Version version() {
    u32 version;
    vkEnumerateInstanceVersion(&version);
    return Version::from_vulkan(version);
  }

  std::vector<VkLayerProperties> layers() {
    return checked_enumerate<VkLayerProperties>(
      vkEnumerateInstanceLayerProperties
    );
  }  
  std::vector<VkExtensionProperties> extensions(const char* layer) {
    return checked_enumerate<VkExtensionProperties>(
      vkEnumerateInstanceExtensionProperties, nullptr 
    );
  }
}