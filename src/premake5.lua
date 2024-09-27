-- Engine code

includedirs( "." );

project "engine"
	local sources = { 
		"./**.cpp",
		"./**.hpp",
	}

	kind "StaticLib"
	location "."

	files( sources )

	includedirs("../external/ode/include");
	includedirs{"../external/cereal-1.3.2/include/"}
	includedirs {
		'../external/assimp/_config_headers/',
		'../external/assimp/_config_headers/assimp/', -- Location of assimp's config.h, for a template see include/assimp/config.h.in
		'../external/assimp/assimp/include/',
	}

--EOF
