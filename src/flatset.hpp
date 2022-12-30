#pragma once 

#include <vector>
#include <algorithm> 

// Set implemented over a contiguous sorted array
template<typename T>
class FlatSet {
  std::vector<T> m_data;

  void sort() {
    std::sort(m_data.begin(), m_data.end());
  }

  void remove_duplicates() {
    std::sort(m_data.begin(), m_data.end());
    m_data.erase(
      std::unique(m_data.begin(), m_data.end()),
      m_data.end()
    );
  }
public:
  using const_iterator = std::vector<T>::const_iterator;

  FlatSet() {}
  FlatSet(std::vector<T>&& vector): m_data(vector) {
    remove_duplicates();
  }
  FlatSet(std::initializer_list<T> list): m_data(list) {
    remove_duplicates();
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
      sort();
    }
  }
  void add(const T& value) { 
    add(value); 
  }

  void append(const std::vector<T>& values) {
    m_data.insert(m_data.end(), values.begin(), values.end());
    remove_duplicates();
  }
  void append(std::initializer_list<T> values) {
    m_data.insert(m_data.end(), values.begin(), values.end());
    remove_duplicates();
  }
};