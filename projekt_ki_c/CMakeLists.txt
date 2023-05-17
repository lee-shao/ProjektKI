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

add_executable(unit_test src/unittest.c src/board.c src/game.c src/network.c)
target_include_directories(unit_test PRIVATE include)
target_compile_options(unit_test PRIVATE -Wall -g) # -O2
#threads because we can hehe
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(unit_test PRIVATE Threads::Threads)
target_link_libraries(unit_test PRIVATE m)

# Testing
enable_testing ()
add_test(NAME Stellung_0 COMMAND $<TARGET_FILE:unit_test> -s 0)
add_test(NAME Stellung_1 COMMAND $<TARGET_FILE:unit_test> -s 1)
add_test(NAME Stellung_2 COMMAND $<TARGET_FILE:unit_test> -s 2)
add_test(NAME Stellung_3 COMMAND $<TARGET_FILE:unit_test> -s 3)
