project "IlluminoEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ipch.h"
	pchsource "src/ipch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",

		"%{IncludeDir.optick}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.assimp}",
	}

	links
	{		
        "d3dcompiler",
        "dxguid",
        "d3d12",
        "dxgi",
		
		"optick",
		"assimp",
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ILLUMINO_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ILLUMINO_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ILLUMINO_DIST"
		runtime "Release"
		optimize "on"
