set(SCRIPT_PATH "${CURRENT_INSTALLED_DIR}/share/qtbase")
include("${SCRIPT_PATH}/qt_install_submodule.cmake")

# Qt's FindWebView2.cmake only looks for the static loader shipped by the NuGet
# SDK, which vcpkg's webview2 port does not build on dynamic triplets.
set(${PORT}_PATCHES find-webview2-import-lib.patch)


vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
FEATURES
    "webengine"     CMAKE_REQUIRE_FIND_PACKAGE_WebEngineCore
    "webview2"      FEATURE_webview_webview2_plugin
INVERTED_FEATURES
    "webengine"     CMAKE_DISABLE_FIND_PACKAGE_WebEngineCore
)

qt_install_submodule(PATCHES    ${${PORT}_PATCHES}
                     CONFIGURE_OPTIONS ${FEATURE_OPTIONS}
                     CONFIGURE_OPTIONS_MAYBE_UNUSED
                        CMAKE_REQUIRE_FIND_PACKAGE_WebEngineCore
                    )
