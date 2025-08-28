# LutinoDB

LutinoDB is a lightweight, in-memory database engine written in C++20. It is designed for fast, simple, and efficient data management without external dependencies. LutinoDB supports basic SQL-like operations, table management, and data persistence to files.

## Features
- In-memory table and database management
- Basic SQL-like command support (SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, etc.)
- Table and column aliasing
- Data persistence via file save/load
- Written in modern C++ (C++20)
- Minimal external dependencies

## Build Instructions

### Requirements
- C++20 compatible compiler (MSVC, GCC, Clang, etc.)
- [CMake](https://cmake.org/) >= 3.8
- [Ninja](https://ninja-build.org/) (recommended)

### Build Steps
```sh
mkdir build
cd build
cmake -G Ninja ..
ninja
```

## Usage
- Include the headers and link the generated library in your project.
- Use the `Database`, `Table`, and `view` classes to manage data.
- Refer to the source code and comments for API details.

## File Structure
- `ltn_db.h` / `ltn_db.cpp`: Core database engine
- `node.hpp`: Node (row) structure
- `ltn_String.h`: Custom string class
- `define.h`: Common definitions

## License
This project is released under the MIT License.

## Author
- pochi0701
