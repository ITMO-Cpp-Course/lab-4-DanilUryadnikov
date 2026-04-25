#include "lab4/resource/resource_manager.hpp"
#include <memory>

namespace lab4::resource{
    std::shared_ptr<FileHandle> ResourceManager::getFile(const std::string& filename, FileHandle::Mode mode){
        auto it = cache_.find(filename);
        if(it != cache_.end()){
            auto ptr = it->second.lock();
            if(ptr){
                return ptr;
            }
            else{
                cache_.erase(it);
            }
        }
    auto ptr = std::make_shared<FileHandle>(filename,mode);
    cache_[filename] = ptr;
    return ptr;
    }
    void ResourceManager::removeFromCache(const std::string& filename) {
    cache_.erase(filename);
}
}