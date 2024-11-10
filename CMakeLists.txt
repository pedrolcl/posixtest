cmake_minimum_required(VERSION 3.16)

project(testdup LANGUAGES C)

add_executable(testdup main.c)

enable_testing()

add_test(NAME testdup1 COMMAND testdup 290000000)
add_test(NAME testdup2 COMMAND testdup 80000000)
add_test(NAME testdup3 COMMAND testdup 5000000)
add_test(NAME testdup4 COMMAND testdup 700000)
add_test(NAME testdup5 COMMAND testdup 30000)
