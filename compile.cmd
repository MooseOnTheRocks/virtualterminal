call cl /EHsc /MD src/main/*.cpp /Feterm.exe /I ".\ext\include\SDL2" /link /LIBPATH:".\ext\lib\SDL2\x86" SDL2.lib SDL2main.lib /SUBSYSTEM:CONSOLE