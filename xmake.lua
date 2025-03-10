add_rules("mode.debug", "mode.release")

set_languages("c++17")
set_optimize("none")

add_requires("libsdl3")
add_requires("libsdl3_image")
add_requires("glm")

target("PixL-Engine")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/*.cpp")
    add_packages("libsdl3", "libsdl3_image" , "glm")