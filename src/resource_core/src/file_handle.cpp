#include "lab4/resource/file_handle.hpp"
#include "lab4/resource/error.hpp"
#include <fcntl.h> 
#include <unistd.h>  
#include <cerrno>  
#include <cstring>

namespace lab4::resource{
    void FileHandle::open_file(const std::string& filename, Mode mode){
        int flags = 0;
        switch (mode) {
            case Mode::Read:
                flags = O_RDONLY;
                break;
            case Mode::Write:
                flags = O_WRONLY | O_CREAT | O_TRUNC;
                break;
            case Mode::Append:
                flags = O_WRONLY | O_CREAT | O_APPEND;
                break;
            default:
                throw ResourceError("Unknown file open mode");
        }
        fd_ = ::open(filename.c_str(), flags, 0644);
        if (fd_ == -1){
            std::string error_msg = "Failed to open file: " + std::string(strerror(errno));
            throw ResourceError(error_msg);
        }
    }
    FileHandle::FileHandle(const std::string& filename, Mode mode):fd_(-1){
        open_file(filename, mode);
    }
    FileHandle::~FileHandle(){
        if(fd_ != -1){
            ::close(fd_);
        }
    }
    void FileHandle::close(){
        if(fd_ != -1){
            ::close(fd_);
            fd_ = -1;
        }
    }
    bool FileHandle::is_open() const noexcept{
        if(fd_ != -1){
            return true;
        }
        return false;
    }
    int FileHandle::fd() const noexcept{
        return fd_;
    }
    FileHandle::FileHandle(FileHandle&& other) noexcept : fd_(other.fd_){
        other.fd_ = -1;
    }
    FileHandle& FileHandle::operator=(FileHandle&& other) noexcept{
        if (this != &other){
            close();
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }
}