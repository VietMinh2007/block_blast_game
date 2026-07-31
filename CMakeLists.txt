cmake_minimum_required(VERSION 3.16)
project(BlockBlast)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# /utf-8 chỉ hợp lệ với MSVC (cl.exe). Với MinGW/g++ (mingw32-make) thì bỏ qua,
# vì GCC vốn đã hiểu UTF-8 mặc định — truyền /utf-8 vào g++ sẽ bị hiểu nhầm
# thành tên file và gây lỗi linker như "No such file or directory".
if(MSVC)
    add_compile_options(/utf-8)
endif()

# --- Đường dẫn tới SFML ---
# Nếu bạn cài SFML ở nơi khác C:/SFML-3.1.0, hãy:
#  1) Sửa dòng SFML_DIR bên dưới cho đúng, HOẶC
#  2) Truyền -DSFML_DIR="D:/duong/dan/toi/SFML/lib/cmake/SFML" khi cấu hình CMake
#     (trong VS Code với extension CMake Tools: mở settings.json và thêm vào "cmake.configureArgs")
if(NOT DEFINED SFML_DIR)
    set(SFML_DIR "C:/SFML-3.1.0/lib/cmake/SFML" CACHE PATH "Path to SFMLConfig.cmake")
endif()

find_package(SFML COMPONENTS Graphics Window System Audio REQUIRED)

# Icon file .exe: trên Windows, app.rc nhúng assets/icon.ico vào file thực thi
# (icon hiển thị trong Explorer/taskbar). Trên các hệ điều hành khác, icon lúc
# chạy vẫn được đặt qua window.setIcon() trong main.cpp bằng assets/icon.png.
set(APP_ICON_RESOURCE "")
if(WIN32)
    set(APP_ICON_RESOURCE app.rc)
endif()

add_executable(blockblast
    src/main.cpp
    src/Grid.cpp
    src/Placement.cpp
    src/Score.cpp
    src/Sound.cpp
    src/Roast.cpp
    src/Spawn.cpp
    ${APP_ICON_RESOURCE}
)

target_link_libraries(blockblast PRIVATE SFML::Graphics SFML::Window SFML::System SFML::Audio)

# Copy thư mục assets ra cùng thư mục với file thực thi (chạy lại mỗi lần build)
add_custom_command(TARGET blockblast POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:blockblast>/assets)

# Trên Windows, các file .dll của SFML cần nằm cùng chỗ với .exe để chạy được.
# Dòng dưới tự copy toàn bộ .dll từ thư mục bin của SFML sang thư mục build.
if(WIN32)
    get_filename_component(SFML_CMAKE_DIR "${SFML_DIR}" DIRECTORY) # .../lib/cmake
    get_filename_component(SFML_LIB_DIR "${SFML_CMAKE_DIR}" DIRECTORY)  # .../lib
    get_filename_component(SFML_ROOT_DIR "${SFML_LIB_DIR}" DIRECTORY)   # .../SFML-x.x.x
    if(EXISTS "${SFML_ROOT_DIR}/bin" AND NOT "${SFML_ROOT_DIR}" MATCHES "msys64|MSYS2")
        add_custom_command(TARGET blockblast POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SFML_ROOT_DIR}/bin" $<TARGET_FILE_DIR:blockblast>)
    endif()
endif()
