#pragma once
#include "types.hpp"
#include "flatset.hpp"

#include <iostream>
#include <compare>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>


struct Error : std::exception {
  VkResult res;

  Error(VkResult res) : res(res) {}
  ~Error() {}
  
  const char* what() { 
    return string_VkResult(res); 
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


struct Instance;

// Instance Builder
struct InstanceDesc {
  FlatSet<const char*> layers;
  FlatSet<const char*> extensions;

  // (Optional) Check if layers and extensions are supported
  const InstanceDesc& check_support(std::ostream& log = std::cout) const;
  Instance build() const; 
};

// Entry point for accessing the Vulkan API
struct Instance {
  VkInstance handle;

  Instance(Instance&&) = delete;

  Instance(VkInstance handle): handle(handle) {}
  ~Instance() { vkDestroyInstance(handle, nullptr); }
};
