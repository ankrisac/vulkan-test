#include <iostream>
#include <string>

#include "types.hpp"
#include "common.hpp"
#include "flatset.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef DEBUG 
const bool DEBUG_ENABLED = true;
#else 
const bool DEBUG_ENABLED = false;
#endif 

FlatSet<const char*> glfwExtensions() {
  u32 count = 0;
  const char** ext = glfwGetRequiredInstanceExtensions(&count);
  return FlatSet<const char*> { 
    { ext, ext + count }
  };
}

bool vulkan_test() {
  if(!glfwInit()) {
    std::cerr << "GLFW init failed" << std::endl;
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(500, 500, "Hello GLFW!", NULL, NULL);

  try {
    auto vulkan = Instance::Builder {
      .extensions = glfwExtensions()
    }
    .enable_validation(DEBUG_ENABLED)
    .check_support()
    .build();

    VkSurfaceKHR surface;

    Error::check(glfwCreateWindowSurface(vulkan.handle, window, nullptr, &surface)); 

    for (auto device : vulkan.get_physical_devices()) {
      std::cout << "Device[" << device.properties().deviceName << "]" << std::endl;

      for (auto family : device.get_queue_families()) {
        std::cout << "- ";
        if (family.can_present(surface) && family.has_graphics()) {
          std::cout << "Good! ";      
        }
        std::cout << "Queue[" 
                  << family.properties.queueCount 
                  << " : "
                  << string_VkQueueFlags(family.properties.queueFlags) 
                  << "]" << std::endl;
      }
    }

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
  vulkan_test();
  return EXIT_SUCCESS;
}