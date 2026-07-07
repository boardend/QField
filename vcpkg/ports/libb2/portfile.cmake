vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO BLAKE2/libb2
    REF 2c5142f12a2cd52f3ee0a43e50a3a76f75badf85
    SHA512 cf29cf9391ae37a978eb6618de6f856f3defa622b8f56c2d5a519ab34fd5e4d91f3bb868601a44e9c9164a2992e80dde188ccc4d1605dffbdf93687336226f8d
    HEAD_REF master
)

set(OPTIONS)
if(CMAKE_HOST_WIN32 OR VCPKG_TARGET_IS_IOS OR VCPKG_TARGET_IS_ANDROID)
    # --enable-native (default) runs AX_EXT, which probes the CPU via cpuid.
    # On cross builds the probe yields "unknown", so configure crashes with
    # `0xunknown: value too great for base`. -march=native is meaningless when
    # cross-compiling anyway, so disable it. (requires cpuid)
    set(OPTIONS --disable-native)
endif()

vcpkg_configure_make(
    AUTOCONFIG
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        ax_cv_check_cflags___O3=no # see https://github.com/microsoft/vcpkg/pull/17912#issuecomment-840514179
        ${OPTIONS}
)
vcpkg_install_make()
vcpkg_fixup_pkgconfig()


file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_copy_pdbs()

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
