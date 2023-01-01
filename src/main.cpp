#include <iostream>
#include <string>

#include <algorithm>

#include "types.hpp"
#include "nameset.hpp"
#include "common.hpp"

#include <GLFW/glfw3.h>

NameSet glfw_extensions() {
  u32 count = 0;
  const char** ext = glfwGetRequiredInstanceExtensions(&count);
  return { ext, ext + count };
}

VkInstance create_instance(bool validation_enabled) {
  NameSet layers = {};
  NameSet extensions = glfw_extensions();

  if (validation_enabled) {
    layers.add("VK_LAYER_KHRONOS_validation");
    extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  layers.supported(Instance::layers());
  extensions.supported(Instance::extensions());

  VkApplicationInfo app_info {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "Hello Vulkan",
    .applicationVersion = 1,
    .pEngineName = "Vulkan-Test",
    .engineVersion = 1,
    .apiVersion = VK_API_VERSION_1_2
  };
  VkInstanceCreateInfo info {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
    .pNext = &DebugLog::create_info,
    .enabledLayerCount = layers.count(),
    .ppEnabledLayerNames = layers.names(),
    .enabledExtensionCount = extensions.count(),
    .ppEnabledExtensionNames = extensions.names()
  };

  VkInstance handle;
  Error::check(vkCreateInstance(&info, nullptr, &handle));
  return handle;
}

struct VulkanState {
  VkInstance instance;
  Option<DebugLog> log;

  VkDevice device;
  VkQueue queue;

  void init(bool validation_enabled) {
    instance = { create_instance(validation_enabled) };

    if(validation_enabled) {
      log = DebugLog {instance};
    }
  }
  void uninit(){
    log.reset();
    vkDestroyInstance(instance, nullptr);
  }
};

int main() {
#ifdef DEBUG 
  const bool VALIDATION_ENABLED = true;
#else 
  const bool VALIDATION_ENABLED = false;
#endif 

  if(!glfwInit()) {
    std::cout << "GLFW init failed";
    return EXIT_FAILURE;
  }
  glfwSetErrorCallback(
    [](int err_code, const char* desc) {
      std::cout 
        << "GLFW [" << err_code << "] : " 
        << desc << std::endl;
    }
  );

  if(!glfwVulkanSupported()) {
    std::cerr << "Vulkan not supported" << std::endl;
  }
  else {
    std::cout 
      << "Vulkan "
      << Instance::version() << std::endl;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(500, 500, "Hello Vulkan", nullptr, nullptr);

  VulkanState state;
  state.init(VALIDATION_ENABLED);

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  state.uninit();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}