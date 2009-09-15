-- vim:ts=4:st=4:sts=4:et:
--

solution "hippoenv_libs"
    configurations { "Debug", "Release" }
    
    dofile("nspr/premake4.lua")
