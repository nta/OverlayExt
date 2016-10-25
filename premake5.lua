dofile('ayria_premake.lua')

ayria_plugin 'OverlayExt'
	links { 'd3d9', 'minhook' }

    includedirs
    {
        'vendor/minhook/include',
    }

group 'vendor'
project 'minhook'
    language 'C'
    kind 'StaticLib'

    includedirs
    {
        'vendor/minhook/include'
    }

    files
    {
        'vendor/minhook/src/*.c'
    }

    filter 'platforms:Win32'
        files 'vendor/minhook/src/HDE/hde32.c'

    filter 'platforms:Win64'
        files 'vendor/minhook/src/HDE/hde64.c'