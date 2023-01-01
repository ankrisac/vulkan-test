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
    if (res != VK_SUCCESS) {
      std::cerr << "Vulkan Error : " << res << std::endl;
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

struct DebugLog {
  VkInstance parent = nullptr;
  VkDebugUtilsMessengerEXT handle = nullptr;

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

  DebugLog(VkInstance parent);
  ~DebugLog();

  DebugLog(DebugLog&& other);
  DebugLog& operator=(DebugLog&& other) = default;

  DebugLog(const DebugLog&) = delete;
  DebugLog& operator=(const DebugLog&) = delete;
};

namespace Instance {
  Version version();
  std::vector<VkLayerProperties> layers();
  std::vector<VkExtensionProperties> extensions(const char* layer = nullptr);
}
