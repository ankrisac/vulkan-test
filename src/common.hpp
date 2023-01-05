#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "types.hpp"

#include <iostream>

struct Error : std::exception {
  VkResult res;

  Error(VkResult res) : res(res) {}
  ~Error() {}
  
  const char* what() { 
    return string_VkResult(res); 
  }

  static void check(VkResult res) {
    if(res != VK_SUCCESS) {
      std::cerr << "Vulkan Error : " << string_VkResult(res) << std::endl;
      throw Error(res);
    }
  }
};

struct Version {
  u32 major   = 0;
  u32 minor   = 0;
  u32 patch   = 0;
  u32 variant = 0;

  static Version from_vulkan(u32 ver) {
    return {
      .major   = VK_API_VERSION_MAJOR(ver),
      .minor   = VK_API_VERSION_MINOR(ver),
      .patch   = VK_API_VERSION_PATCH(ver),
      .variant = VK_API_VERSION_VARIANT(ver),
    };
  }

  // Spaceship operator doesn't default impl ==, != unless explicitly defaulted
  friend std::strong_ordering operator<=>(const Version&, const Version&);
  friend bool operator==(const Version&, const Version&) = default;
  friend std::ostream& operator<<(std::ostream& out, const Version& ver);
};




/// @tparam Enumerator (Args..., u32*, T*) -> VkResult
template<typename T, typename Enumerator, typename... Args>
std::vector<T> checked_enumerate(Enumerator fn, Args... args) {
  u32 count = 0;
  Error::check(fn(args..., &count, nullptr));

  std::vector<T> data { count };
  Error::check(fn(args..., &count, data.data()));
  return data;
}

/// @tparam Enumerator (Args..., u32*, T*) -> void
template<typename T, typename Enumerator, typename... Args>
std::vector<T> unchecked_enumerate(Enumerator fn, Args... args) {
  u32 count = 0;
  fn(args..., &count, nullptr);

  std::vector<T> data { count };
  fn(args..., &count, data.data());
  return data;
}

template<typename U, typename T, typename Fn>
std::vector<U> map(const std::vector<T>& in, Fn fn) {
  std::vector<U> out(in.size());
  for(size_t i = 0; i < in.size(); i++) {
    out[i] = fn(in[i]);
  }
  return out;
}

struct DebugLog {
  VkInstance parent;
  VkDebugUtilsMessengerEXT handle;
  bool validation_enabled;

  static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void*                                       pUserData
  ) {
    std::cout 
      << "\033[31mValidation\033[0m: "
      << pCallbackData->pMessage
      << std::endl;
    return VK_FALSE;
  }
  constexpr static VkDebugUtilsMessengerCreateInfoEXT create_info = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT, 
    .messageSeverity 
      = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
    .messageType
      = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
    .pfnUserCallback = &debug_callback
  };

  DebugLog(VkInstance, bool);
  ~DebugLog();

  DebugLog(DebugLog&&);
  DebugLog& operator=(DebugLog&&) = default;
};

struct PhysicalDevice;
struct QueueFamily;

struct Device;
struct Queue;

struct Surface;

struct Instance {
  using Handle = VkInstance;
  using CreateInfo = VkInstanceCreateInfo;

  Handle handle;

  Instance(CreateInfo info) {
    Error::check(vkCreateInstance(&info, nullptr, &handle));
  }
  ~Instance() {
    vkDestroyInstance(handle, nullptr);
  }

  // Move only - cannot be copied
  Instance(Instance&& other) {
    handle = other.handle;
    other.handle = VK_NULL_HANDLE;
  }  
  Instance& operator=(Instance&&) = default;


  static Version version();
  static std::vector<VkLayerProperties> layers();
  static std::vector<VkExtensionProperties> extensions(const char* layer = nullptr);

  std::vector<PhysicalDevice> devices() const;
};

struct PhysicalDevice {
  using Handle = VkPhysicalDevice;
  Handle handle;

  using Features = VkPhysicalDeviceFeatures;
  using Properties = VkPhysicalDeviceProperties;

  Features features();
  Properties properties();

  std::vector<VkExtensionProperties> extensions(const char* layer_name = nullptr);
  std::vector<QueueFamily> queue_families();

  bool can_present(const QueueFamily& family, const Surface& surface);

  Device create_device(VkDeviceCreateInfo& info);
};

struct QueueFamily {
  using Handle = VkQueueFamilyProperties;
  using Index = u32;

  Handle handle;
  Index index;

  bool has(VkQueueFlags flags) {
    return handle.queueFlags & flags;
  } 
  bool has_graphics() {
    return has(VK_QUEUE_GRAPHICS_BIT);
  }
  bool has_compute() {
    return has(VK_QUEUE_COMPUTE_BIT);
  }
};

struct Device {
  using Handle = VkDevice;
  Handle handle;

  Device(Handle handle);
  ~Device();

  Device(Device&& other);
  Device& operator=(Device&& other) = default;
 
  Queue get_queue(QueueFamily::Index family, u32 index) const;
};

struct Queue {
  using Handle = VkQueue;
  Handle handle;
};

struct Surface {
  using Handle = VkSurfaceKHR;
  Handle handle;
  VkInstance instance;

  Surface(Handle handle, VkInstance instance);
  ~Surface();

  Surface(Surface&& other);
  Surface& operator=(Surface&& other) = default;

  using Limits = VkSurfaceCapabilitiesKHR;
  using Format = VkSurfaceFormatKHR;
  using PresentMode = VkPresentModeKHR;

  Limits get_limits(const PhysicalDevice& device) const;
  std::vector<Format> get_formats(const PhysicalDevice& device) const;
  std::vector<PresentMode> get_present_modes(const PhysicalDevice& device) const;

  bool compatible_with(const PhysicalDevice& device);
  void setup_swapchain(VkDevice device, VkSwapchainCreateInfoKHR config);
};

struct Swapchain {
  using Handle = VkSwapchainKHR;
  using CreateInfo = VkSwapchainCreateInfoKHR;

  struct Config {
    VkSurfaceFormatKHR format;
    VkPresentModeKHR present_mode;
    VkExtent2D size;
  };

  Handle handle;
  const Device& device;

  ~Swapchain() { 
    vkDestroySwapchainKHR(device.handle, handle, nullptr); 
  } 
};