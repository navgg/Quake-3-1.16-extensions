rem set "gamedir=C:\Games\Quake 3 Arena 1.16n clean\"
set "gamedir=D:\Games\Quake 3\"
rem set "gamedir=C:\Games\Quake\Quake 3 Arena 1.16n Vanilla\Vanilla Quake III Arena 1.16n\"

set "pakname={X-Mod}.pk3"

bin_nt\7za a -tzip -mx0 %pakname% %CD%\baseq3\vm

move "%pakname%" "%gamedir%baseq3\%pakname%"

rem START /D "%gamedir%" /MAX "" quake3.exe +set sv_pure 0 +connect localhost:27960
START /D "%gamedir%" /MAX "" quake3.exe +set sv_pure 0 +devmap 13box +set g_gametype 4 
rem +addbot anarki 3 r +addbot bones b 3 +addbot mynx r 3 +addbot lucy 3 b
rem +devmap q3dm1 +set cg_debugEvents 1 +set cg_debugDelag 1 +set cg_showMiss 1 +set cg_drawBBox 1
rem +cg_debuganim 1
rem +devmap q3dm1