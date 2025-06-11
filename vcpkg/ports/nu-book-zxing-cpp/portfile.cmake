vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO zxing-cpp/zxing-cpp
    REF e2da6952ca475b1c4d15b94306996de0a8b68e81
    SHA512 8a21427a30da03ef0abd0514f4bcbff6fa95ccd74d0dd27f08c55a514c6150bce122b56ed8ffd465abbad52ab75910570ed8e2ad1e539511b508590272cd1836
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_BLACKBOX_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_fixup_pkgconfig()

vcpkg_cmake_config_fixup(
    CONFIG_PATH lib/cmake/ZXing
    PACKAGE_NAME ZXing
)

file(READ "${CURRENT_PACKAGES_DIR}/share/ZXing/ZXingConfig.cmake" _contents)
file(WRITE "${CURRENT_PACKAGES_DIR}/share/ZXing/ZXingConfig.cmake" "
include(CMakeFindDependencyMacro)
find_dependency(Threads)
${_contents}")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
