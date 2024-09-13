workspace "AsciiTerraRPG"
	architecture "x64"
configurations
	{
		"Debug",
		"Release"
	}

	outputdir = "%{cfg.buildcfg}_%{cfg.system}_%{cfg.architecture}"

project "SpNetCommon"
	location "SpNetCommon"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	cppdialect "C++17"

	targetdir("bin/" .. outputdir)
	objdir("bin-int/" .. outputdir)

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs 
	{
		"%{prj.name}/vendor/asio/include",
		"%{prj.name}/src"
	}

	filter "system:windows"
	
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"

		defines 
		{
			"BUILD_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"
		defines 
		{
			"BUILD_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "_SPNET_DEBUG"
		runtime "Debug"
		buildoptions "/MD"
		symbols "on"
		optimize "off"
	
	filter "configurations:Release"
		defines "_SPNET_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"

project "AsciiTerra_Client"
		location "AsciiTerra_Client"
		kind "ConsoleApp"
		language "C++"
		staticruntime "on"
		cppdialect "C++17"

		targetdir("bin/" .. outputdir)
		objdir("bin-int/" .. outputdir)

		files 
		{
			"%{prj.name}/**.h",
			"%{prj.name}/**.cpp"
		}

		includedirs 
		{	
			"SpNetCommon/src",
			"SpNetCommon/vendor/asio/include"
		}

		links 
		{"SpNetCommon"}

	filter "system:windows"
	
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"

		defines 
		{
			"BUILD_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"
		defines 
		{
			"BUILD_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "BUILD_DEBUG"
		runtime "Debug"
		buildoptions "/MD"
		symbols "on"
		optimize "off"
	
	filter "configurations:Release"
		defines "BUILD_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"

project "AsciiTerra_Server"
		location "AsciiTerra_Server"
		kind "ConsoleApp"
		language "C++"
		staticruntime "on"
		cppdialect "C++17"

		targetdir("bin/" .. outputdir)
		objdir("bin-int/" .. outputdir)

		files 
		{
			"%{prj.name}/**.h",
			"%{prj.name}/**.cpp"
		}

		includedirs 
		{	
			"SpNetCommon/src",
			"SpNetCommon/vendor/asio/include"
		}

		links 
		{"SpNetCommon"}

	filter "system:windows"
	
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"

		defines 
		{
			"BUILD_PLATFORM_WINDOWS"
		}

	filter "system:linux"
		staticruntime "On"
		systemversion "latest"
		buildoptions "/MD"
		defines 
		{
			"BUILD_PLATFORM_LINUX"
		}

	filter "configurations:Debug"
		defines "BUILD_DEBUG"
		runtime "Debug"
		buildoptions "/MD"
		symbols "on"
		optimize "off"
	
	filter "configurations:Release"
		defines "BUILD_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"