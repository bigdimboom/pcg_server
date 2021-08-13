# CMakeList.txt : CMake project for PcgServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("PcgServer")
set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tinyply CONFIG REQUIRED)

find_path(CPP_HTTPLIB_INCLUDE_DIRS "httplib.h")

# Add source to this project's executable.
add_executable (PcgServer test.main.cpp "dimcli/cli.h" "dimcli/cli.cpp" "ply/ply_reader.h" "ply/ply_utils.h")
target_link_libraries(PcgServer nlohmann_json nlohmann_json::nlohmann_json spdlog::spdlog spdlog::spdlog_header_only tinyply)
target_include_directories(PcgServer PRIVATE ${CPP_HTTPLIB_INCLUDE_DIRS})

# TODO: Add tests and install targets if needed.
