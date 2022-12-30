#include <iostream>
#include <string>

#include <GLFW/glfw3.h>

#include "types.hpp"
#include "instance.hpp"


#include "flatset.hpp"

void vulkan_info() {
  u32 vk_version = 0;
  vkEnumerateInstanceVersion(&vk_version);

  std::cout 
    << "Vulkan: " << Version::from_vulkan(vk_version) << std::endl;
}

FlatSet<const char*> glfwExtensions() {
  u32 count = 0;
  const char** ext = glfwGetRequiredInstanceExtensions(&count);
  return FlatSet<const char*> { 
    { ext, ext + count } 
  };
}


#ifdef DEBUG 
const bool DEBUG_ENABLED = true;
#else 
const bool DEBUG_ENABLED = false;
#endif 


bool vulkan_test() {
  if(!glfwInit()) {
    std::cerr << "GLFW init failed" << std::endl;
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(500, 500, "Hello GLFW!", NULL, NULL);

  vulkan_info();

  try {
    auto instance = InstanceDesc { 
        .layers = FlatSet<const char*>{}
          .add_if("VK_KHRONOS_validation", DEBUG_ENABLED),
        .extensions = glfwExtensions()
          .add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) 
      }
      .check_support()
      .build();
  
    while(!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }
  catch (Error& err) {
    std::cout << err.what() << std::endl;
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return true;
}

int main() { 
  //vulkan_test();
  return EXIT_SUCCESS;
}