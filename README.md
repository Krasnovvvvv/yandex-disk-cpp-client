# ☁️ Yandex Disk C++ Client

![GitHub Tag](https://img.shields.io/github/v/tag/Krasnovvvvv/yandex-disk-cpp-client?color=blue)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://Krasnovvvvv.github.io/yandex-disk-cpp-client/)
[![License](https://img.shields.io/github/license/Krasnovvvvv/yandex-disk-cpp-client)](LICENSE)

![Ubuntu](https://img.shields.io/badge/Ubuntu-20.04+-orange?logo=ubuntu)
![Windows](https://img.shields.io/badge/Windows-10+-blue?logo=windows)
![macOS](https://img.shields.io/badge/macOS-12+-silver?logo=apple)

![CI Ubuntu](https://img.shields.io/github/actions/workflow/status/Krasnovvvvv/yandex-disk-cpp-client/ci.yml?branch=main&label=Ubuntu&logo=ubuntu)
![CI Windows](https://img.shields.io/github/actions/workflow/status/Krasnovvvvv/yandex-disk-cpp-client/ci.yml?branch=main&label=Windows&logo=windows)
![CI macOS](https://img.shields.io/github/actions/workflow/status/Krasnovvvvv/yandex-disk-cpp-client/ci.yml?branch=main&label=macOS&logo=apple)

Modern C++ client library for the Yandex.Disk REST API  
A simple, lightweight, and efficient static library for integrating Yandex.Disk cloud storage into C++ projects across platforms

🔥 Now available on vcpkg – install in one command!

You can quickly install the library using [vcpkg](#-installation-via-vcpkg)

---

## ✨ Features

- **Wide API Coverage:**  
  Upload and download files and directories, manage directories, move and rename resources, handle trash operations, publish and unpublish files, and retrieve public links

- **Robust File Management:**  
  Recursive upload and download of directories, existence checks, and detailed resource information retrieval

- **Trash Support:**  
  List trash contents, restore files or folders to original locations, delete individual items, or empty the entire trash

- **Search Functionality:**  
  Find files and folders by name both on the disk and in the trash, with recursive search support

- **Cross-Platform Compatibility:**  
  Works on Windows, Linux, and macOS with Unicode path support

- **Minimal Dependencies:**  
  Uses only `libcurl` for HTTP communication and `nlohmann/json` for JSON parsing

- **Easy Integration:**  
  Provided as a static library with a clean public header interface for straightforward use in CMake projects

---

## 📁 Project Structure

```text
yandex-disk-cpp-client/
├── .github/workflows/       # GitHub Actions CI
├── cmake/                   # Package config templates
├── docs/                    # Generated Doxygen documentation and extra docs
├── examples/                # Example usage programs
├── include/                 # Public headers
├── src/                     # Library source files
├── tests/                   # Unit tests and test CMake configuration
├── CMakeLists.txt           # Root build configuration
├── vcpkg.json               # vcpkg manifest dependencies
├── README.md                # This file
├── LICENSE                  # License file
└── .gitignore               # Git ignore rules
```

---

## 🚀 Quick Start

### 🛠️ Prerequisites

- C++17 compatible compiler
- CMake 3.28 or newer
- [vcpkg](https://github.com/microsoft/vcpkg)
- Yandex.Disk OAuth token with the required permissions

### 🔑 OAuth Token

Set the `YADISK_TOKEN` environment variable before running examples or your own application.

Detailed step-by-step instructions for registering a Yandex OAuth application and obtaining a token are available in [`docs/OAUTH_TOKEN.md`](docs/OAUTH_TOKEN.md).

### ⚡ Build and Run Example

```sh
git clone https://github.com/Krasnovvvvv/yandex-disk-cpp-client.git
cd yandex-disk-cpp-client

cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_EXAMPLES=ON

cmake --build build
```

Then run an example executable, for example:

```sh
./build/example_basic_usage
```

On Windows:

```powershell
.\build\Debug\example_basic_usage.exe
```

---

## 📦 Installation via vcpkg

You can install **yandex-disk-cpp-client** directly from the official [vcpkg](https://github.com/microsoft/vcpkg) registry.

#### Install

```bash
vcpkg install yandex-disk-cpp-client
```

If you use a specific triplet:

```bash
vcpkg install yandex-disk-cpp-client:x64-mingw-static
```

#### Use in CMake

```cmake
find_package(yandex-disk-cpp-client CONFIG REQUIRED)
target_link_libraries(example PRIVATE yandex-disk-cpp-client::yandex-disk-cpp-client)
```

#### Notes

- `libcurl` and `nlohmann-json` are resolved automatically via vcpkg
- Works on Windows, Linux, and macOS
- Useful both for local development and CI environments

---

## 📖 Example Usage

```cpp
#include "YandexDiskClient.h"
#include <cstdlib>
#include <iostream>

int main() {
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    auto quota = yandex.getQuotaInfo();
    std::cout << yandex.formatQuotaInfo(quota) << std::endl;

    auto list = yandex.getResourceList("/");
    std::cout << yandex.formatResourceList(list) << std::endl;

    return 0;
}
```

> For more examples, see [`examples/`](examples/)

---

## 🧭 API Coverage

| Category | Methods |
|----------|---------|
| Disk info | `getQuotaInfo()`, `formatQuotaInfo()` |
| Resource listing | `getResourceList(path)`, `formatResourceList(json)`, `getResourceInfo(path)` |
| File operations | `uploadFile(disk_path, local_path)`, `downloadFile(disk_path, local_path)` |
| Directory operations | `createDirectory(path)`, `uploadDirectory(disk_path, local_path)`, `downloadDirectory(disk_path, local_path)` |
| Resource management | `deleteFileOrDir(path)`, `moveFileOrDir(from, to, overwrite)`, `renameFileOrDir(path, new_name, overwrite)`, `exists(path)` |
| Public access | `publish(path)`, `unpublish(path)`, `getPublicDownloadLink(path)` |
| Trash operations | `getTrashResourceList(path)`, `formatTrashResourceList(json)`, `restoreFromTrash(path)`, `deleteFromTrash(path)`, `emptyTrash()` |
| Search | `findResourcePathByName(name, start_path)`, `findTrashPathByName(name)` |

---

## 🧪 Tests

Unit tests are located in the [`tests/`](tests/) directory and are built through a dedicated `tests/CMakeLists.txt`.

To build and run tests:

```sh
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake \
  -DBUILD_TESTS=ON \
  -DBUILD_EXAMPLES=OFF

cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 📚 Documentation

API documentation is generated with Doxygen and published from the `docs/` directory.

Online documentation:
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://Krasnovvvvv.github.io/yandex-disk-cpp-client/)

---

## 📦 Dependencies

- [libcurl](https://curl.se/libcurl/) — HTTP requests
- [nlohmann/json](https://github.com/nlohmann/json) — JSON parsing
- [GoogleTest](https://github.com/google/googletest) — unit tests only

If you use vcpkg manifest mode, dependencies are installed automatically from `vcpkg.json`.

---

## 🤝 Contribution

Contributions, bug reports, and feature requests are welcome.  
Please open an issue or submit a pull request.

---

## 📝 License

This project is licensed under the MIT License — see [LICENSE](LICENSE)