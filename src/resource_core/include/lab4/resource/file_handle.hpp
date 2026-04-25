#pragma once
#include <string>

namespace lab4::resource
{
class FileHandle
{
  public:
    enum class Mode
    {
        Read,
        Write,
        Append
    };
    explicit FileHandle(const std::string& filename, Mode mode);
    ~FileHandle();
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;
    bool is_open() const noexcept;
    void close();
    int fd() const noexcept;

  private:
    int fd_;
    void open_file(const std::string& filename, Mode mode);
};
} // namespace lab4::resource