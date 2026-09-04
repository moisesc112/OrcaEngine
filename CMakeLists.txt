# CMakeList.txt : CMake project for OrcaEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OrcaEngine")

# Add source to this project's executable.
add_executable (OrcaEngine "src/main.cpp" 
						   "src/Application.cpp" 
						   "src/Swapchain.cpp" 
						   "src/VulkanContext.cpp" 
						   "src/OrcaEngine.cpp" 
						   "src/OrcaEngine.h" "src/VulkanUtils.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OrcaEngine PROPERTY CXX_STANDARD 20)
  set_property(TARGET OrcaEngine PROPERTY CXX_STANDARD_REQUIRED ON)
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(external/glm)
add_subdirectory(external/glfw)

target_include_directories(OrcaEngine PRIVATE include)
target_include_directories(OrcaEngine PRIVATE external/stb)
target_include_directories(OrcaEngine PRIVATE external/tinyobjloader)

target_link_libraries(OrcaEngine PRIVATE Vulkan::Vulkan 
										 glm::glm 
										 glfw)

target_include_directories(OrcaEngine PRIVATE ${Vulkan_INCLUDE_DIRS})

# TODO: Add tests and install targets if needed
