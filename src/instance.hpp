#pragma once

#include <iostream>
#include <compare>

#include "common.hpp"
#include "flatset.hpp"

// Entry point for accessing the Vulkan API
struct Instance {
  struct Builder {
    FlatSet<const char*> layers;
    FlatSet<const char*> extensions;

    bool validation_enabled = false;
    
    Builder& enable_validation(bool toggle = true);

    // (Optional) Check if layers and extensions are supported
    const Builder& check_support(std::ostream& log = std::cerr) const;
    Instance build() const; 
  };

  VkInstance handle;
  Option<VkDebugUtilsMessengerEXT> messenger;

  Instance(VkInstance handle) : handle(handle) {}
  ~Instance();

  Instance(Instance&& other);
  Instance& operator=(Instance&&) = default;

  Instance(const Instance&) = delete;
  Instance& operator=(const Instance&) = delete;
};