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

  Surface create_surface(const Instance& instance, GLFWwindow* window) {
    Surface::Handle handle;
    glfwCreateWindowSurface(instance.handle, window, nullptr, &handle);
    return Surface { handle, instance.handle };
  }
}


struct Adapter {
  PhysicalDevice physical_device;
  QueueFamily family;
  NameSet extensions;

  static Adapter from(const Instance& instance, const Surface& surface) {
    constexpr auto score = [](PhysicalDevice device) {
      auto prop = device.properties();
      return (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 10 : 1);
    };
    constexpr auto compare_device = [](PhysicalDevice left, PhysicalDevice right) {
      return score(left) > score(right);
    };

    auto devices = instance.devices();
    std::sort(devices.begin(), devices.end(), compare_device);

    NameSet extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    std::cout << "Looking for Adapter" << std::endl;
    for (auto device : devices) {
      std::cout 
        << "Adapter[" 
        << device.properties().deviceName
        << "]" << std::endl;

      if(!extensions.supported(device.extensions())) continue;

      for(auto family : device.queue_families()) {
        if(device.can_present(family, surface) && family.has_graphics()) {    
          std::cout << "" << std::endl;

          return Adapter {
            .physical_device = device,
            .family = family,
            .extensions = extensions,
          };
        }
      }

      std::cout << "Rejected" << std::endl;
    }

    throw std::runtime_error("No suitable Adapter found");
  }

  std::pair<Device, Queue> request_device() {
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
      .enabledExtensionCount = extensions.count(),
      .ppEnabledExtensionNames = extensions.names()
    };

    auto device = physical_device.create_device(info);
    auto queue = device.get_queue(family.index, 0);

    return { std::move(device), queue };
  }
};



Instance create_instance(bool validation_enabled) {
  NameSet layers = {};
  NameSet extensions = GLFW::extensions();

  if(validation_enabled) {
    layers.add("VK_LAYER_KHRONOS_validation");
    extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  if(!layers.supported(Instance::layers())
  || !extensions.supported(Instance::extensions())) {
    throw std::runtime_error("Cannot create instance");
  }

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
  return Instance { info };
}

VkExtent2D swapchain_size(const VkSurfaceCapabilitiesKHR capabilities) {
  auto current_extent = capabilities.currentExtent;
  u32 special_dim = std::numeric_limits<u32>::max();

  if (current_extent.width == special_dim 
  && current_extent.height == special_dim) {
    
  }
  return current_extent;
}

VkSwapchainCreateInfoKHR swapchain_config(
  const Surface&        surface, 
  const PhysicalDevice& device
) {
  auto limits = surface.get_limits(device);

  VkSwapchainCreateInfoKHR info {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface.handle, 
    .minImageCount = limits.minImageCount + 1,     

    .presentMode = VK_PRESENT_MODE_FIFO_KHR
  };

  for(auto mode : surface.get_present_modes(device)) {
    if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      info.presentMode = mode;
      break; 
    }
  }

  for(auto fmt : surface.get_formats(device)) {
    if(fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    && fmt.format == VK_FORMAT_B8G8R8_SRGB) {
      info.imageFormat = fmt.format;
      info.imageColorSpace = fmt.colorSpace;
      break;
    }
  }

  return info;
}

struct VulkanState {
  Instance instance;
  Option<DebugLog> log;
  Surface surface;

  Device device;
  Queue queue;

  static VulkanState make(bool validation_enabled, GLFWwindow* window) {
    Instance instance = create_instance(validation_enabled);

    Option<DebugLog> log;
    if (validation_enabled) 
      log = std::move(DebugLog { instance.handle, validation_enabled });
    Surface surface = GLFW::create_surface(instance, window);

    auto adapter = Adapter::from(instance, surface);
    auto [device, queue] = adapter.request_device();
    
    auto config = swapchain_config(surface, adapter.physical_device);
    //surface.setup_swapchain(device, config);

    return VulkanState {
      .instance = std::move(instance),
      .log = std::move(log),
      .surface = std::move(surface),
      .device = std::move(device),
      .queue = std::move(queue)
    };
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

  if(!glfwVulkanSupported()) {
    std::cerr << "Vulkan not supported" << std::endl;
  }
  else {
    std::cout << "Vulkan " << Instance::version() << std::endl;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(500, 500, "Hello Vulkan", nullptr, nullptr);

  {
    auto state = VulkanState::make(VALIDATION_ENABLED, window);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}

int main() {
  vulkan_test();
  return EXIT_SUCCESS;
}