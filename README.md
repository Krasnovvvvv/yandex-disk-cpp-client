# ☁️ Yandex Disk C++ Client

[![License](https://img.shields.io/github/license/Krasnovvvvv/yandex-disk-cpp-client)](LICENSE)
[![Documentation](https://img.shields.io/badge/docs-online-blue)](https://Krasnovvvvv.github.io/yandex-disk-cpp-client/)
[![GitHub stars](https://img.shields.io/github/stars/Krasnovvvvv/yandex-disk-cpp-client?style=social)](https://github.com/Krasnovvvvv/yandex-disk-cpp-client/stargazers)

![Ubuntu](https://img.shields.io/badge/Ubuntu-20.04+-orange?logo=ubuntu)
![Windows](https://img.shields.io/badge/Windows-10+-blue?logo=windows)
![CI Ubuntu](https://img.shields.io/github/actions/workflow/status/Krasnovvvvv/numerical-methods-in-physics/ci.yml?branch=main&label=Ubuntu&logo=ubuntu)
![CI Windows](https://img.shields.io/github/actions/workflow/status/Krasnovvvvv/numerical-methods-in-physics/ci.yml?branch=main&label=Windows&logo=windows)

Modern C++ client library for the Yandex.Disk REST API.  
A simple, lightweight, and efficient static library for integrating Yandex.Disk cloud storage into C++ projects across platforms

---

## ✨ Features

- **Full API Coverage:**  
  Upload and download files and directories, manage directories, move and rename resources, handle trash operations, publish/unpublish files, and retrieve public download links

- **Robust File Management:**  
  Recursive upload/download of directories, existence checks, and detailed resource information retrieval

- **Trash Support:**  
  List trash contents, restore files/folders to original locations, delete individual items or empty the entire trash

- **Search Functionality:**  
  Find files and folders by name both on the disk and in the trash, supporting recursive search and multiple matches

- **Cross-Platform Compatibility:**  
  Works on Windows, Linux, and macOS with support for Unicode paths

- **Minimal Dependencies:**  
  Depends only on `libcurl` for HTTP communication and `nlohmann/json` for JSON parsing

- **Easy Integration:**  
  Provided as a static library with a clean header interface for straightforward inclusion in your projects

---

## 📁 Project Structure
```
yandex-disk-cpp-client/
├── docs                     # Generated documentation via Doxygen
├── examples/                # Example usage programs
├── include/                 # Public headers (YandexDiskClient.h)
├── src/                     # Library source files (YandexDiskClient.cpp)
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── LICENSE                  # License file
├── .gitignore               # Git ignore rules
```

---

## 🚀 Quick Start

### 🛠️ Prerequisites

- C++17 compatible compiler (GCC, Clang, MSVC)
- CMake 3.14 or newer
- libcurl development files
- nlohmann/json (header-only, managed via CMake)
- Environment variable `YADISK_TOKEN` with your Yandex.Disk OAuth token **(full disk access)**

### ⚡ Build and Run Example

```sh
git clone https://github.com/Krasnovvvvv/yandex-disk-cpp-client.git
cd yandex-disk-cpp-client
mkdir build && cd build
cmake ..
cmake --build .
./yandex_disk_example
```

### 📖 Example Usage

```cpp
#include "YandexDiskClient.h"
#include <cstdlib> // for getenv

int main() {
    const char* token = std::getenv("YADISK_TOKEN");
    if (!token) {
        std::cerr << "Please set YADISK_TOKEN environment variable." << std::endl;
        return 1;
    }

    YandexDiskClient yandex(token);

    // Upload a file
    yandex.uploadFile("/backup/data.zip", "C:/local/data.zip");

    // List root directory contents
    auto list = yandex.getResourceList("/");
    std::cout << yandex.formatResourceList(list) << std::endl;

    return 0;
}
```
> For more examples, see `examples/`

---

## 🧭 API Overview

| Function                                 | Description                                               |
|------------------------------------------|-----------------------------------------------------------|
| `getQuotaInfo()`                         | Retrieve disk quota info (total, used, trash size)        |
| `getResourceList(path)`                  | List files and folders at a given disk path               |
| `getResourceInfo(path)`                  | Get detailed info about a file or folder                  |
| `uploadFile(disk_path, local_path)`      | Upload a local file to disk                               |
| `downloadFile(disk_path, local_path)`    | Download a file from disk to local path                   |
| `uploadDirectory(disk_path, local_path)` | Recursively upload a directory                            |
| `downloadDirectory(disk_path, local_path)`| Recursively download a directory                         |
| `deleteFileOrDir(path)`                  | Delete a file or directory                                |
| `createDirectory(path)`                  | Create a directory                                        |
| `moveFileOrDir(from, to, overwrite)`     | Move or rename a file or directory                        |
| `publish(path)`                          | Publish a file or folder (make public)                    |
| `unpublish(path)`                        | Remove public access                                      |
| `getPublicDownloadLink(path)`            | Get public download URL                                   |
| `exists(path)`                           | Check if a file or folder exists                          |
| `getTrashResourceList(path)`             | List contents of trash                                    |
| `restoreFromTrash(path)`                 | Restore file/folder from trash to original location       |
| `deleteFromTrash(path)`                  | Permanently delete from trash                             |
| `emptyTrash()`                           | Empty the entire trash                                    |
| `findTrashPathByName(name)`              | Find all trash items by name                              |
| `findResourcePathByName(name, start_path)`| Find all disk items by name, recursively                 |

---

## 📦 Dependencies

- [libcurl](https://curl.se/libcurl/) — for HTTP requests
- [nlohmann/json](https://github.com/nlohmann/json) — for JSON parsing

> These dependencies are automatically handled via CMake (assuming installed on your system or via package managers like vcpkg)

---

## 📚 Documentation

Full API documentation is generated using Doxygen and available in the `docs/` folder  
You can also access the online documentation via GitHub Pages [![Documentation](https://img.shields.io/badge/docs-online-blue)](https://Krasnovvvvv.github.io/yandex-disk-cpp-client/)

---

## 🤝 Contribution

Contributions, bug reports, and feature requests are welcome!  
Please open issues or pull requests on the GitHub repository

---

## 📝 License

This project is licensed under the MIT License — see [![License](https://img.shields.io/github/license/Krasnovvvvv/yandex-disk-cpp-client)](LICENSE)

