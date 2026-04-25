#pragma once
#include "file_handle.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace lab4::resource
{
class ResourceManager
{
  public:
    std::shared_ptr<FileHandle> getFile(const std::string& filename, FileHandle::Mode mode);
    void removeFromCache(const std::string& filename);

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};
} // namespace lab4::resource