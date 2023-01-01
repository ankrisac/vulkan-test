#include <iostream>
#include <string>

#include <algorithm>

#include "types.hpp"
#include "nameset.hpp"
#include "common.hpp"

#include <GLFW/glfw3.h>

namespace GLFW {
  NameSet extensions() {
    u32 count = 0;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);
    return { ext, ext + count };
  }
}

Instance create_instance(bool validation_enabled) {
  NameSet layers = {};
  NameSet extensions = GLFW::extensions();

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
  Instance::CreateInfo info {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
    .pNext = &DebugLog::create_info,
    .enabledLayerCount = layers.count(),
    .ppEnabledLayerNames = layers.names(),
    .enabledExtensionCount = extensions.count(),
    .ppEnabledExtensionNames = extensions.names()
  };
  Instance out;
  out.init(info);
  return out;
}

struct Adapter {
  PhysicalDevice physical_device;
  QueueFamily family;

  void request_device(VkDevice& device, VkQueue& queue) {
    f32 priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = family.index,
      .queueCount = 1,
      .pQueuePriorities = &priority,
    };

    VkDeviceCreateInfo info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_info,
    };
    vkCreateDevice(physical_device.handle, &info, nullptr, &device);
    vkGetDeviceQueue(device, family.index, 0, &queue);
  }
};

Adapter find_adapter(Instance instance, VkSurfaceKHR surface) {
  constexpr auto score = [](PhysicalDevice device) {
    auto prop = device.properties();
    return (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 10 : 1);
  };
  constexpr auto compare_device = [](PhysicalDevice left, PhysicalDevice right) {
    return score(left) > score(right);
  };

  auto devices = instance.devices();
  std::sort(devices.begin(), devices.end(), compare_device);

  for(auto device : devices) {
    for (auto family : device.queue_families()) {
      if(device.can_present(family, surface) && family.has_graphics()) {
        std::cout 
          << "Adapter[" 
          << device.properties().deviceName 
          << "]" << std::endl;
      
        return { device, family };
      }
    }
  }

  throw std::runtime_error("No suitable Adapter found");
}

struct VulkanState {
  Instance instance;
  Option<DebugLog> log;

  VkSurfaceKHR surface;
  VkDevice device;
  VkQueue queue;

  void init(bool validation_enabled, GLFWwindow* window) {
    instance = create_instance(validation_enabled);

    if(validation_enabled) {
      log = DebugLog {instance.handle};
    }

    Error::check(glfwCreateWindowSurface(instance.handle, window, nullptr, &surface));
  
    find_adapter(instance, surface)
      .request_device(device, queue);
  }
  void uninit(){
    vkDestroyDevice(device, nullptr);

    log.reset();
    instance.uninit();
  }
};

int vulkan_test() {
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

  if (!glfwVulkanSupported()) {
    std::cerr << "Vulkan not supported" << std::endl;
  }
  else {
    std::cout << "Vulkan " << Instance::version() << std::endl;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(500, 500, "Hello Vulkan", nullptr, nullptr);

  VulkanState state;
  state.init(VALIDATION_ENABLED, window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  state.uninit();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}

int main() {
  vulkan_test();
  return EXIT_SUCCESS;
}