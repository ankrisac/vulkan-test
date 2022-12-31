#include "common.hpp"

using Self = Instance;

Self::Builder& Self::Builder::enable_validation(bool toggle) {
  validation_enabled = toggle;

  if(toggle) {
    layers.add("VK_LAYER_KHRONOS_validation");
    extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return *this;
}

template<typename T, typename S, typename GetName>
bool any_missing(
  std::ostream&         log,
  const std::vector<T>& available, 
  const std::vector<S>& required,
  GetName               view
) {
  bool missing = false;

  for (auto name : required) {
    bool found = false;

    for(auto avail : available) {
      if(std::strcmp(name, view(avail)) == 0) {
        found = true;
        break;
      }   
    }

    if(!found) {
      log << name << " not supported!" << std::endl; 
      
      missing = true;
    }

  }
  return missing;
}

const Self::Builder& Self::Builder::check_support(std::ostream& log) const {
  using Layer = VkLayerProperties;
  any_missing(
    log,
    enumerate<Layer>(vkEnumerateInstanceLayerProperties),
    layers.vec(),
    [](Layer layer) { return layer.layerName; } 
  );

  using Extension = VkExtensionProperties;
  any_missing(
    log,
    enumerate<Extension>(vkEnumerateInstanceExtensionProperties, nullptr),
    extensions.vec(),
    [](Extension ext) { return ext.extensionName; }
  );

  return *this;
}



VkDebugUtilsMessengerCreateInfoEXT make_debug_info() {
  auto callback = [](
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, 
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void*                                       pUserData
  ) {
    // Todo : Print out label, and other info.

    std::cout 
      << "\033[31mValidation\033[0m: " 
      << pCallbackData->pMessage
      << std::endl;
    return VK_FALSE;
  };

  return VkDebugUtilsMessengerCreateInfoEXT {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity 
      = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType    
      = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
    .pfnUserCallback = +callback
  };
}

Self Self::Builder::build() const {
  VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "VulkanTest",
    .applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 0),

    .pEngineName = "sgfx",
    .engineVersion = VK_MAKE_API_VERSION(0, 0, 0, 0),
    .apiVersion = VK_API_VERSION_1_2
  };

  VkInstanceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &app_info, 

    .enabledLayerCount = static_cast<u32>(layers.size()),
    .ppEnabledLayerNames = layers.data(),

    .enabledExtensionCount = static_cast<u32>(extensions.size()),
    .ppEnabledExtensionNames = extensions.data()
  };

  VkDebugUtilsMessengerCreateInfoEXT debug_info = make_debug_info();
  if (validation_enabled) {
    info.pNext = &debug_info;
  }

  VkInstance handle;
  Error::check(vkCreateInstance(&info, nullptr, &handle));
  Self out { handle };

  if(validation_enabled) {
    auto create = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(handle, "vkCreateDebugUtilsMessengerEXT")
    );

    VkDebugUtilsMessengerEXT debug_handle;
    create(handle, &debug_info, nullptr, &debug_handle);
    out.messenger = debug_handle;
  }
  return out;
}

Self::~Instance() {
  if (messenger.has_value()) {
    auto destroy = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(handle, "vkDestroyDebugUtilsMessengerEXT")
    );
    destroy(handle, messenger.value(), nullptr);
  }
  vkDestroyInstance(handle, nullptr);
}

Self::Instance(Self&& other)
: handle(other.handle), messenger(other.messenger) 
{
  // Cannot use default impl, as we need to avoid calling the destructor
  other.handle = VK_NULL_HANDLE;
  other.messenger = {};
}


std::vector<PhysicalDevice> Self::get_physical_devices() {
  auto devices = enumerate<VkPhysicalDevice>(
    vkEnumeratePhysicalDevices, handle
  );

  std::vector<PhysicalDevice> out { devices.size() };
  for (size_t i = 0; i < devices.size(); i++) {
    out[i] = PhysicalDevice {
      .parent = this,
      .handle = devices[i]
    };
  }
  return out;
}