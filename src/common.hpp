#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <GLFW/glfw3.h>

#include "types.hpp"
#include "flatset.hpp"

#include <iostream>

struct Error : std::exception {
  VkResult res;

  Error(VkResult res) : res(res) {}
  ~Error() {}
  
  const char* what() { 
    return string_VkResult(res); 
  }

  
  static void check(VkResult res) {
    if (res != VK_SUCCESS) {
      std::cout << "Vulkan Error : " << res << std::endl;
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
std::vector<T> enumerate(Enumerator fn, Args... args) {
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



// Entry point for accessing the Vulkan API
struct PhysicalDevice;
struct Surface;

struct Instance {
  struct Builder {
    FlatSet<const char*> layers;
    FlatSet<const char*> extensions;

    bool validation_enabled = false;
    
    Builder& enable_validation(bool toggle = true);

    // (Optional) Check if layers and extensions are supported
    const Builder& check_support(std::ostream& log = std::cerr) const;
    Instance build() const; 
  };

  VkInstance handle;
  Option<VkDebugUtilsMessengerEXT> messenger;

  // Ownership semantics : Move only, no copying

  Instance(VkInstance handle) : handle(handle) {}
  ~Instance();

  Instance(Instance&& other);
  Instance& operator=(Instance&&) = default;

  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;



  std::vector<PhysicalDevice> get_physical_devices();
};


struct QueueFamily;

// Request using Instance::get_physical_devices()
struct PhysicalDevice {
  Instance* parent;
  VkPhysicalDevice handle;

  using Properties = VkPhysicalDeviceProperties;
  using Features   = VkPhysicalDeviceFeatures;

  Features features() const;
  Properties properties() const;

  std::vector<QueueFamily> get_queue_families();
};

// Request using PhysicalDevice::get_queue_families()

struct QueueFamily {
  using Index = u32;

  PhysicalDevice* parent;
  Index index;

  VkQueueFamilyProperties properties;    

  bool can_present(VkSurfaceKHR surface) {
    VkBool32 supported = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(parent->handle, index, surface, &supported);
    return supported;
  }

  bool has(VkQueueFlagBits flag) {
    return properties.queueFlags & flag;  
  }

  bool has_graphics() { return has(VK_QUEUE_GRAPHICS_BIT); }
  bool has_compute() { return has(VK_QUEUE_COMPUTE_BIT); }
};
