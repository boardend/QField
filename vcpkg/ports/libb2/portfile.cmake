vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO BLAKE2/libb2
    REF 643decfbf8ae600c3387686754d74c84144950d1
    SHA512 4727633659900d7726a7c64dddd88be1952265b0857ed5c8e69e0e03ad0d702dd3e6ff948fa357957be0ebdfcd8f523d4e60bd3c141229bb43c5faaf933bdc6b
    HEAD_REF master
)

vcpkg_list(SET options)

#if(CMAKE_HOST_WIN32)
    vcpkg_list(APPEND options "--disable-native")
#endif()

if(VCPKG_TARGET_IS_IOS)
    # see https://github.com/microsoft/vcpkg/pull/17912#issuecomment-840514179
    vcpkg_list(APPEND options "ax_cv_check_cflags___O3=no")
endif()

vcpkg_configure_make(
    AUTOCONFIG
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        ${options}
)
vcpkg_install_make()
vcpkg_fixup_pkgconfig()


file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_copy_pdbs()

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
