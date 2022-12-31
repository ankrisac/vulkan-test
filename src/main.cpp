#include <iostream>
#include <string>

#include "types.hpp"

#include <GLFW/glfw3.h>

#include "instance.hpp"
#include "flatset.hpp"

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