#include "test_filesystem.h"

#include <iostream>
#include <format>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace fs = boost::filesystem;

namespace test_filesystem {

void test() {
    fs::path dir = fs::temp_directory_path() / "boost_demo";
    std::cout << std::format("Directory path: {}\n", dir.string());
    fs::create_directory(dir);

    fs::path file_path = dir / "text.txt";
    std::cout << std::format("File path: {}\n", file_path.string());

    // запись в файл через boost::filesystem::ofstream
    fs::ofstream ofs(file_path);
    ofs << "Hello from Boost.Filesystem!\n";
    ofs << "Line 2\n";
    ofs.close();

    if (fs::exists(file_path)) {
        std::cout << std::format("Size: {}\n", fs::file_size(file_path));
        std::cout << std::format("Extension: {}\n", file_path.extension().string());

        // чтение обратно через boost::filesystem::ifstream
        fs::ifstream ifs(file_path);
        std::string line;
        while (std::getline(ifs, line)) {
            std::cout << std::format("  > {}\n", line);
        }
    }

    boost::system::error_code ec;
    for (auto& e : fs::directory_iterator(dir, ec)) {
        std::cout << std::format("Entry: {}\n", e.path().filename().string());
    }

    fs::remove_all(dir);
    std::cout << std::format("Remove: {}\n", dir.string());
}

}
