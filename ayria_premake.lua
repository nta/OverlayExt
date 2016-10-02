function ayria_plugin(name)
	workspace(name)
		configurations { "Debug", "Release" }
		platforms { "Win32", "x64" }

		location "build/"

		flags { "StaticRuntime", "No64BitChecks", "Symbols", "Unicode" }

		flags { "NoIncrementalLink", "NoEditAndContinue" }

		filter "platforms:Win32"
			architecture "x32"

		filter "platforms:Win64"
			architecture "x64"

		filter "configurations:Debug"
			defines "_DEBUG"

		filter "configurations:Release"
			defines "NDEBUG"
			optimize "Speed"

		-- reset filter
		filter {}

		targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}/"
		objdir "obj/%{cfg.platform}/%{cfg.buildcfg}/"

	project(name)
		language "C++"
		kind "SharedLib"

		targetname(name)

		defines { "AYRIA_EXTENSION_NAME=" .. name, "BUILDHOST=" .. (os.getenv("COMPUTERNAME") or os.getenv("HOSTNAME")) }

		filter "platforms:Win32"
			targetextension ".Ayria32"

		filter "platforms:Win64"
			targetextension ".Ayria64"

		filter {}

		pchsource "src/StdInc.cpp"
		pchheader "StdInc.h"

		includedirs { "include/" }

		files
		{
			"src/**.cpp",
			"src/**.cc",
			"src/**.c",
			"src/**.h",
			"include/**.hpp",
			"include/**.h",
		}

		postbuildcommands { "if exist \"$(SolutionDir)..\\postbuild.cmd\" call \"$(SolutionDir)..\\postbuild.cmd\" \"$(TargetPath)\" \"$(TargetFileName)\" %{prj.name}" }
end