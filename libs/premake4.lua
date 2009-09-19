-- vim:ts=4:st=4:sts=4:et:
--

solution "hippoenv_libs"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" } 
    
    targetdir "bin"
    location  "."

dofile("nspr/premake4.lua")
dofile("spidermonkey/premake4.lua")
dofile("iconv/premake4.lua")
dofile("zlib/premake4.lua")
