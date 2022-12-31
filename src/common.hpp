#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "types.hpp"
#include <iostream>

void vk_try(const VkResult res);

struct Error : std::exception {
  VkResult res;

  Error(VkResult res) : res(res) {}
  ~Error() {}
  
  const char* what() { 
    return string_VkResult(res); 
  }

  static void check(const VkResult res) {
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
