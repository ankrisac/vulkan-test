#pragma once 

#include <vector>
#include <algorithm> 

// Set implemented over a contiguous sorted array

template<typename T>
class FlatSet {
  std::vector<T> m_data;
public:
  using const_iterator = std::vector<T>::const_iterator;

  FlatSet(std::initializer_list<T> list) {
    m_data = list;
    std::sort(m_data.begin(), m_data.end());
  }  

  const_iterator begin() const { 
    return m_data.begin(); 
  }
  const_iterator end() const { 
    return m_data.end(); 
  } 

  const T* data() const {
    return m_data.data();
  }  
  size_t size() const {
    return m_data.size();
  }
  size_t capacity() const {
    return m_data.capacity();
  }

  void reserve(size_t size) {
    m_data.reserve(size);
  }

  bool contains(const T& value) const {
    return std::binary_search(m_data.begin(), m_data.end(), value);
  }

  void add(T&& value) { 
    if(!contains(value)) {
      m_data.push_back(value); 
      std::sort(m_data.begin(), m_data.end());
    }
  }
  void add(const T& value) { 
    add(value); 
  }
};