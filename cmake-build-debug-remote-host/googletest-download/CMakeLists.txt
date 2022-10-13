cmake_minimum_required(VERSION 3.8)

project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main
        SOURCE_DIR "/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/googletest-src"
        BINARY_DIR "/tmp/tmp.IyJU8EsJ7j/cmake-build-debug-remote-host/googletest-build"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
        )
