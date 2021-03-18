cmake_minimum_required(VERSION 3.1)
project(mysql)

set(MYSQL_PLUGIN_VERSION "R42")

option (BUILD_TESTS "Build tests" OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/libs/cmake")

if(UNIX)
	#force 32bit compilation and hide non-exported symbols
	set(CMAKE_CXX_FLAGS "-m32 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-m32 -fvisibility=hidden")
	set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
endif()

include(libs/conan-cmake/conan.cmake)
function(add_conan_lib pkg_name conan_pkg)
	conan_check(REQUIRED)

	conan_cmake_run(
		REQUIRES ${conan_pkg}
		ARCH x86
		BASIC_SETUP CMAKE_TARGETS
		BUILD missing
	)

	set_target_properties(${pkg_name} PROPERTIES IMPORTED_GLOBAL TRUE)
endfunction()

add_conan_lib(CONAN_PKG::mariadb-connector-c mariadb-connector-c/3.1.11)

conan_check(REQUIRED)
conan_cmake_run(
	REQUIRES boost/1.73.0
	ARCH x86
	BASIC_SETUP CMAKE_TARGETS
	BUILD missing
	OPTIONS boost:zlib=False
)
set_target_properties(CONAN_PKG::boost PROPERTIES IMPORTED_GLOBAL TRUE)

set(CONAN_HAS_FMT TRUE CACHE INTERNAL FALSE)
add_conan_lib(CONAN_PKG::fmt fmt/7.1.3)

add_subdirectory(libs/log-core)
add_subdirectory(src)
