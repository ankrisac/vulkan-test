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



DebugLog::DebugLog(VkInstance parent, bool validation_enabled)
: parent(parent), validation_enabled(validation_enabled)
{
  if(validation_enabled) {
    auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(parent, "vkCreateDebugUtilsMessengerEXT")
    );
    create(parent, &create_info, nullptr, &handle);
  }
}
DebugLog::~DebugLog() 
{
  if(validation_enabled) {
    if(handle != VK_NULL_HANDLE) {  
      auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(parent, "vkDestroyUtilsMessengerEXT")
      );
      destroy(parent, handle, nullptr);
    }
  }
}
DebugLog::DebugLog(DebugLog&& other) {
  handle = other.handle;
  parent = other.parent;

  other.handle = VK_NULL_HANDLE;
  other.parent = VK_NULL_HANDLE;
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
std::vector<PhysicalDevice> Instance::devices() const {
  return map<PhysicalDevice>(
    checked_enumerate<VkPhysicalDevice>(
      vkEnumeratePhysicalDevices, handle 
    ),
    [](auto device){ return PhysicalDevice { device }; }
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
std::vector<VkExtensionProperties> PhysicalDevice::extensions(const char* layer_name) {
  return checked_enumerate<VkExtensionProperties>(
    vkEnumerateDeviceExtensionProperties, handle, layer_name
  );
}


bool PhysicalDevice::can_present(const QueueFamily& family, const Surface& surface) {
  VkBool32 supported = VK_FALSE;
  Error::check(vkGetPhysicalDeviceSurfaceSupportKHR(
    handle, family.index, surface.handle, &supported
  ));
  return static_cast<bool>(supported);
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
Device PhysicalDevice::create_device(VkDeviceCreateInfo& info) {
  VkDevice device;
  Error::check(vkCreateDevice(handle, &info, nullptr, &device));
  return Device { device };
}

Device::Device(Handle handle): handle(handle) {

}
Device::Device(Device&& other) {
  handle = other.handle;
  other.handle = VK_NULL_HANDLE;
}
Device::~Device() {
  vkDestroyDevice(handle, nullptr);
}

Queue Device::get_queue(QueueFamily::Index family, u32 index) const {
  Queue out;
  vkGetDeviceQueue(handle, family, index, &out.handle);
  return out;
}



Surface::Surface(Handle handle, VkInstance instance)
: handle(handle), instance(instance) {}
Surface::~Surface() {
  if(handle != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, handle, nullptr); 
  }
}
Surface::Surface(Surface&& other) {
  handle = other.handle;
  instance = other.instance;

  other.handle = VK_NULL_HANDLE;
  other.instance = VK_NULL_HANDLE;
};


Surface::Limits Surface::get_limits(const PhysicalDevice &device) const
{
  Limits out;
  Error::check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    device.handle, handle, &out
  ));
  return out;
}

std::vector<Surface::Format> Surface::get_formats(const PhysicalDevice &device) const {
  return checked_enumerate<Format>(
    vkGetPhysicalDeviceSurfaceFormatsKHR, device.handle, handle
  );
}

std::vector<Surface::PresentMode> Surface::get_present_modes(const PhysicalDevice &device) const {
  return checked_enumerate<PresentMode>(
    vkGetPhysicalDeviceSurfacePresentModesKHR, device.handle, handle
  );
}

bool Surface::compatible_with(const PhysicalDevice& device) {
  // VK_PRESENT_MODE_FIFO_KHR is required (from Vulkan 1.3 spec)
  // But I don't know if that assumes compatibility of the surface

  return !get_formats(device).empty() 
      && !get_present_modes(device).empty();
}

void Surface::setup_swapchain(VkDevice device, VkSwapchainCreateInfoKHR config) {
}
