project(client)

set(INCLUDE_DIRS
    ${ANDROID_SYSTEM_SERVICE_EXAMPLE}/include
    ${ANDROID_SYSTEM_SERVICE_EXAMPLE}/NDKBuild/include
    )

add_executable(${PROJECT_NAME} client.cpp)

target_include_directories(client PRIVATE ${INCLUDE_DIRS})