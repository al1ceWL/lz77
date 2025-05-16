cmake_minimum_required(VERSION 3.10)  # Минимальная версия CMake
project(LZ77Compressor)               # Название проекта

# Создание статической библиотеки
add_library(lz77 STATIC lz77.c)
target_include_directories(lz77 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) # Чтобы main.c видел lz77.h

# Создание исполняемого файла
add_executable(lz77_compressor
    main.c
)
# Связывает исполняемый файл lz77_compressor с библиотекой lz77
target_link_libraries(lz77_compressor lz77)