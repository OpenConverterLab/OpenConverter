cmake_minimum_required(VERSION 3.10)

# Enable testing
enable_testing()

# Add GTest from 3rd_party
add_subdirectory(${CMAKE_SOURCE_DIR}/../3rd_party/gtest ${CMAKE_BINARY_DIR}/gtest)

# Add test executable
add_executable(transcoder_test transcoder_test.cpp)

# Set C++17 for std::filesystem
target_compile_features(transcoder_test PRIVATE cxx_std_17)
target_compile_definitions(transcoder_test PRIVATE 
    _LIBCPP_ENABLE_CXX17_REMOVED_FEATURES
    GTEST_HAS_PTHREAD=1
    TEST_MEDIA_PATH="${CMAKE_SOURCE_DIR}/build/tests/media"
)

# Include directories
target_include_directories(transcoder_test PRIVATE
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_SOURCE_DIR}/transcoder/include
    ${CMAKE_SOURCE_DIR}/engine/include
)

# Link against GTest and core library
target_link_libraries(transcoder_test
    PRIVATE
    gtest_main
    OpenConverterCore
)

# Copy GTest libraries to test directory
add_custom_command(TARGET transcoder_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:gtest>
        $<TARGET_FILE:gtest_main>
        $<TARGET_FILE:gmock>
        $<TARGET_FILE:gmock_main>
        $<TARGET_FILE_DIR:transcoder_test>
)

# Add test to CTest
add_test(NAME transcoder_test COMMAND transcoder_test) 