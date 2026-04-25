#include "lab4/resource/error.hpp"
#include "lab4/resource/resource_manager.hpp"
#include <catch2/catch_all.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <unistd.h>

using namespace lab4::resource;
namespace fs = std::filesystem;

static std::string create_temp_file()
{
    std::string name =
        "test_file_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".txt";
    std::ofstream ofs(name);
    ofs << "test content";
    ofs.close();
    return name;
}

// ========== Тесты для ResourceError ==========
TEST_CASE("ResourceError can be thrown and caught", "[error]")
{
    REQUIRE_THROWS_AS(throw ResourceError("test"), ResourceError);
    try
    {
        throw ResourceError("message");
    }
    catch (const ResourceError& e)
    {
        REQUIRE(std::string(e.what()) == "message");
    }
}

// ========== Тесты для FileHandle ==========
TEST_CASE("FileHandle opens file successfully", "[filehandle]")
{
    std::string filename = create_temp_file();
    {
        FileHandle fh(filename, FileHandle::Mode::Read);
        REQUIRE(fh.is_open());
        REQUIRE(fh.fd() >= 0);
    }
    fs::remove(filename);
}

TEST_CASE("FileHandle closes file on destruction", "[filehandle]")
{
    std::string filename = create_temp_file();
    {
        FileHandle fh(filename, FileHandle::Mode::Read);
        REQUIRE(fh.is_open());
    }
    fs::remove(filename);
}

TEST_CASE("FileHandle throws on non-existent file in read mode", "[filehandle]")
{
    REQUIRE_THROWS_AS(FileHandle("non_existent_file_xyz.txt", FileHandle::Mode::Read), ResourceError);
}

TEST_CASE("FileHandle write mode creates file", "[filehandle]")
{
    std::string filename = "write_test.txt";
    {
        FileHandle fh(filename, FileHandle::Mode::Write);
        REQUIRE(fh.is_open());
    }
    REQUIRE(fs::exists(filename));
    fs::remove(filename);
}

TEST_CASE("FileHandle append mode appends data", "[filehandle]")
{
    std::string filename = "append_test.txt";
    {
        FileHandle fh(filename, FileHandle::Mode::Write);
        REQUIRE(fh.is_open());
        ssize_t ret = ::write(fh.fd(), "hello", 5);
        REQUIRE(ret == 5);
    }
    {
        FileHandle fh(filename, FileHandle::Mode::Append);
        ssize_t ret = ::write(fh.fd(), "world", 5);
        REQUIRE(ret == 5);
    }
    std::ifstream ifs(filename);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    REQUIRE(content == "helloworld");
    fs::remove(filename);
}

TEST_CASE("FileHandle move constructor transfers ownership", "[filehandle]")
{
    std::string filename = create_temp_file();
    FileHandle fh1(filename, FileHandle::Mode::Read);
    int fd1 = fh1.fd();
    FileHandle fh2 = std::move(fh1);
    REQUIRE(fh2.is_open());
    REQUIRE(fh2.fd() == fd1);
    REQUIRE_FALSE(fh1.is_open());
    REQUIRE(fh1.fd() == -1);
    fs::remove(filename);
}

TEST_CASE("FileHandle move assignment transfers ownership", "[filehandle]")
{
    std::string filename = create_temp_file();
    FileHandle fh1(filename, FileHandle::Mode::Read);
    FileHandle fh2("dummy.txt", FileHandle::Mode::Write);
    int fd1 = fh1.fd();
    fh2 = std::move(fh1);
    REQUIRE(fh2.is_open());
    REQUIRE(fh2.fd() == fd1);
    REQUIRE_FALSE(fh1.is_open());
    fs::remove(filename);
    fs::remove("dummy.txt");
}

// ========== Тесты для ResourceManager ==========
TEST_CASE("ResourceManager returns same shared_ptr for same file", "[manager]")
{
    ResourceManager rm;
    std::string filename = create_temp_file();
    auto ptr1 = rm.getFile(filename, FileHandle::Mode::Read);
    auto ptr2 = rm.getFile(filename, FileHandle::Mode::Read);
    REQUIRE(ptr1.get() == ptr2.get());
    REQUIRE(ptr1.use_count() == 2);
    fs::remove(filename);
}

TEST_CASE("ResourceManager cache expires after last shared_ptr destroyed", "[manager]")
{
    ResourceManager rm;
    std::string filename = create_temp_file();
    std::weak_ptr<FileHandle> weak;
    {
        auto ptr = rm.getFile(filename, FileHandle::Mode::Read);
        weak = ptr;
        REQUIRE_FALSE(weak.expired());
    }
    REQUIRE(weak.expired());
    auto ptr2 = rm.getFile(filename, FileHandle::Mode::Read);
    REQUIRE_FALSE(weak.lock() == ptr2);
    fs::remove(filename);
}

TEST_CASE("ResourceManager removeFromCache forces new open", "[manager]")
{
    ResourceManager rm;
    std::string filename = create_temp_file();
    auto ptr1 = rm.getFile(filename, FileHandle::Mode::Read);
    rm.removeFromCache(filename);
    auto ptr2 = rm.getFile(filename, FileHandle::Mode::Read);
    REQUIRE(ptr1.get() != ptr2.get());
    fs::remove(filename);
}

TEST_CASE("ResourceManager handles non-existent file gracefully", "[manager]")
{
    ResourceManager rm;
    REQUIRE_THROWS_AS(rm.getFile("missing.txt", FileHandle::Mode::Read), ResourceError);
}