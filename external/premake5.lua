-- Third party projects


includedirs( "glfw/include" );
includedirs( "glew-1.13.0/include" );
includedirs( "glm-0.9.7.1" );


defines { "GLEW_STATIC"}

project ("cereal")
kind "StaticLib"

location "."

filter "*"

files { "cereal-1.3.2/include/**.hpp" }


project( "x-glfw" )
kind "StaticLib"

location "."

filter "system:linux"
defines { "_GLFW_X11=1" }

filter "system:windows"
defines { "_GLFW_WIN32=1" }

filter "*"

files {
    "glfw/src/context.c",
    "glfw/src/egl_context.c",
    "glfw/src/init.c",
    "glfw/src/input.c",
    "glfw/src/internal.h",
    "glfw/src/mappings.h",
    "glfw/src/monitor.c",
    "glfw/src/null_init.c",
    "glfw/src/null_joystick.c",
    "glfw/src/null_joystick.h",
    "glfw/src/null_monitor.c",
    "glfw/src/null_platform.h",
    "glfw/src/null_window.c",
    "glfw/src/platform.c",
    "glfw/src/platform.h",
    "glfw/src/vulkan.c",
    "glfw/src/window.c",
    "glfw/src/osmesa_context.c",
};

filter "system:linux"
files {
    "glfw/src/posix_*",
    "glfw/src/x11_*",
    "glfw/src/xkb_*",
    "glfw/src/glx_*",
    "glfw/src/linux_*",
};
filter "system:windows"
files {
    "glfw/src/win32_*",
    "glfw/src/wgl_*",
};

filter "*"

project( "x-glew" )
kind "StaticLib"

location "."

files { "glew-1.13.0/src/glew.c" }

filter "*"

project( "x-glm" )
kind "Utility"

location "."

files( "glm-0.9.7.1/**.h" )
files( "glm-0.9.7.1/**.hpp" )
files( "glm-0.9.7.1/**.inl" )

include "ode/build"
include "assimp"

project( "imgui" )
kind "StaticLib"

location "."

files ( "imgui/**.h")
files ( "imgui/**.cpp")

--	location "."

--files("assimp/**.lib")
--	links {"assimp/assimp-vc142-mtd"}
--	links {"assimp/zlibstaticd"}

--	files( "assimp/**.h" )
--	files( "assimp/**.hpp" )
--	libdirs {"assimp/"}
--EOF

--include "assimp"
