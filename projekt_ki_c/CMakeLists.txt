cmake_minimum_required (VERSION 3.0)
project(projekt_ki)
set (CMAKE_C_STANDARD 11)

add_executable(projekt_ki src/main.c src/board.c src/game.c src/network.c)
target_include_directories(projekt_ki PRIVATE include)
target_compile_options(projekt_ki PRIVATE -Wall -g)
#threads because we can hehe
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(projekt_ki PRIVATE Threads::Threads)
target_link_libraries(projekt_ki PRIVATE m)
