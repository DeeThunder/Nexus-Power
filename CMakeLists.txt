cmake_minimum_required(VERSION 3.16.0)

# Set the project version manually to avoid Git-related errors in ESP-IDF
set(PROJECT_VER "1.0.0")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Smart-Power-bank)
