#pragma once
#include <vector>
#include <algorithm>

#include "types.hpp"
#include "common.hpp"



class NameSet {
  std::vector<const char*> m_data;

  void sort() {
    std::sort(m_data.begin(), m_data.end());
  }
  void remove_duplicates() {
    sort();
    m_data.erase(
      std::unique(m_data.begin(), m_data.end()), 
      m_data.end()
    );
  }
public:
  template<typename Iter>
  NameSet(Iter begin, Iter end): m_data(begin, end) {
    remove_duplicates();
  }
  NameSet(std::initializer_list<const char*> list): m_data(list) {
    remove_duplicates();
  }

  bool find(const char* name) {
    return std::binary_search(m_data.begin(), m_data.end(), name);
  }

  void add(const char* name) {
    if (!find(name)) {
      m_data.push_back(name);
      remove_duplicates();
    }  
  }
  constexpr u32 count() const {
    return static_cast<u32>(m_data.size());
  }
  constexpr const char* const* names() const {
    return m_data.data();
  }

  template<typename Property, typename Fn>
  bool impl_supported(const std::vector<Property>& available, Fn get_name) const {
    bool any_missing = false;
    for (auto name : m_data) {
      bool found = false;

      for (auto property : available) {
        if (std::strcmp(name, get_name(property)) == 0) {
          found = true;
          break;
        }
      } 

      if(!found) any_missing = true;
      std::cerr
        << (found ? "[X]" : "[ ]") 
        << " : " << name << std::endl; 
    }

    return !any_missing;
  }

  bool supported(const std::vector<VkLayerProperties>& available) const {
    std::cout << "Checking Layer Support" << std::endl;
    bool any_missing = false;
    return impl_supported<VkLayerProperties>(available, 
      [](auto layer) { return layer.layerName; }
    );
  }
  bool supported(const std::vector<VkExtensionProperties>& available) const {
    std::cout << "Checking Extension Support" << std::endl;
    return impl_supported<VkExtensionProperties>(available, 
      [](auto extension) { return extension.extensionName; }
    );
  }
};