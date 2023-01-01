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



Version Instance::version() {
  u32 version;
  vkEnumerateInstanceVersion(&version);
  return Version::from_vulkan(version);
}
std::vector<VkLayerProperties> Instance::layers() {
  return checked_enumerate<VkLayerProperties>(
    vkEnumerateInstanceLayerProperties
  );
}  
std::vector<VkExtensionProperties> Instance::extensions(const char* layer) {
  return checked_enumerate<VkExtensionProperties>(
    vkEnumerateInstanceExtensionProperties, nullptr 
  );
}
std::vector<PhysicalDevice> Instance::devices() {
  return map<PhysicalDevice>(
    checked_enumerate<VkPhysicalDevice>(
      vkEnumeratePhysicalDevices, handle 
    ),
    [](auto device){ return PhysicalDevice { device }; }
  );
}



std::vector<QueueFamily> PhysicalDevice::queue_families() {
  auto families = unchecked_enumerate<VkQueueFamilyProperties>(
    vkGetPhysicalDeviceQueueFamilyProperties, handle
  );

  std::vector<QueueFamily> out { families.size() };
  for(size_t i = 0; i < families.size(); i++) {
    out[i] = {
      .handle = families[i],
      .index = static_cast<QueueFamily::Index>(i)
    };
  }
  return out;
}

bool PhysicalDevice::can_present(const QueueFamily& family, VkSurfaceKHR surface) {
  VkBool32 supported = VK_FALSE;
  Error::check(vkGetPhysicalDeviceSurfaceSupportKHR(
    handle, family.index, surface, &supported
  ));
  return static_cast<bool>(supported);
}

Device PhysicalDevice::create_device(VkDeviceCreateInfo& info) {
  VkDevice device;
  Error::check(vkCreateDevice(handle, &info, nullptr, &device));
  return Device { device };
}

std::vector<VkExtensionProperties> PhysicalDevice::extensions(const char* layer_name) {
  return checked_enumerate<VkExtensionProperties>(
    vkEnumerateDeviceExtensionProperties, handle, layer_name
  );
}

PhysicalDevice::Features PhysicalDevice::features() {
  Features out;
  vkGetPhysicalDeviceFeatures(handle, &out);
  return out;
}
PhysicalDevice::Properties PhysicalDevice::properties() {
  Properties out;
  vkGetPhysicalDeviceProperties(handle, &out);
  return out;
}
