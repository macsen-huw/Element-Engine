workspace "GroupProject"
language "C++"
cppdialect "C++17"

platforms { "x64" }
configurations { "debug", "release" }

flags "NoPCH"
flags "MultiProcessorCompile"

startproject "main"

debugdir "%{wks.location}"
objdir "_build_/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
targetsuffix "-%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
includedirs{"external/cereal-1.3.2/include/"}
includedirs{"external/glm-0.9.7.1/glm"}
includedirs{"external/"}
debugdir "%{wks.location}"
objdir "_build_/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
targetsuffix "-%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"

includedirs{"external/glm-0.9.7.1/glm"}
includedirs{"external/assimp/assimp/include/assimp/"}
includedirs("external/imgui");
--includedirs{"external/ode/include/"}
includedirs{"external/"}



-- Default toolset options
filter "toolset:gcc or toolset:clang"
linkoptions { "-pthread", "-fsanitize=address,undefined"}
buildoptions { "-march=native", "-Wall", "-g3", "-pthread" ,"-Wextra", "-Wfatal-errors", "-O2", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"}

filter "toolset:msc-*"
defines { "_CRT_SECURE_NO_WARNINGS=1" }
defines { "_SCL_SECURE_NO_WARNINGS=1" }
buildoptions { "/utf-8" }

filter "*"

-- default libraries
filter "system:linux"
links "dl"
links "GL"
links "GLX"

filter "system:windows"
links "OpenGL32"

filter "*"

-- default outputs
filter "kind:StaticLib"
targetdir "lib/"

filter "kind:ConsoleApp"
targetdir "bin/"
targetextension ".exe"

filter "*"

--configurations
filter "debug"
symbols "On"
defines { "_DEBUG=1" }

filter "release"
optimize "On"
defines { "NDEBUG=1" }

filter "*"

-- Third party dependencies
include "external"
-- Engine code
include "src"

-- Projects
project "main"
local sources = {
	"main/**.cpp",
	"main/**.hpp",
}

kind "ConsoleApp"
location "main"

files( sources )

links "engine"
links "common"
links "x-glfw"
links "x-glew"

includedirs( "." );
includedirs( "src" );
	links "engine"
	links "common"
	links "x-glfw"
	links "x-glew"
	links "ode"
	links "assimp"
	links "imgui"
	
	includedirs( "." );
	includedirs( "src" );

	--TODO remove?
	includedirs("external/ode/include");
	includedirs{"external/cereal-1.3.2/include/"}
	includedirs {
		'external/assimp/_config_headers/',
		'external/assimp/_config_headers/assimp/', -- Location of assimp's config.h, for a template see include/assimp/config.h.in
		'external/assimp/assimp/include/',
	}

	files( sources )

	-- Link the correct libraries based on configuration
	--filter "debug"
	--	libdirs {"external/ode-0.16.5/build/vs2010/x64/DebugSingleLib"}
	--	links {"ode_single"}

	--filter "release"
	--	libdirs {"external/ode-0.16.5/build/vs2010/x64/ReleaseSingleLib"}
	--	links {"ode_single"}

project "demo"
local sources = {
	"demo/**.cpp",
	"demo/**.hpp",
}

kind "ConsoleApp"
location "demo"

files( sources )

links "engine"
links "common"
links "x-glfw"
links "x-glew"

includedirs( "." );
includedirs( "src" );
	links "engine"
	links "common"
	links "x-glfw"
	links "x-glew"
	links "ode"
	links "assimp"
	links "imgui"
	
	includedirs( "." );
	includedirs( "src" );

	--TODO remove?
	includedirs("external/ode/include");
	includedirs{"external/cereal-1.3.2/include/"}
	includedirs {
		'external/assimp/_config_headers/',
		'external/assimp/_config_headers/assimp/', -- Location of assimp's config.h, for a template see include/assimp/config.h.in
		'external/assimp/assimp/include/',
	}

	files( sources )



project "common"
local sources = {
	"common/**.cpp",
	"common/**.hpp"
}

kind "StaticLib"
location "common"

files( sources )

--EOF
