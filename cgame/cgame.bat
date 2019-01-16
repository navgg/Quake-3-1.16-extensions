@echo off

setlocal
set "PATH=..\..\bin_nt;%PATH%"

mkdir vm
cd vm

set cc=lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_pmove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_slidemove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../cg_consolecmds.c
@if errorlevel 1 goto quit
%cc% ../cg_draw.c
@if errorlevel 1 goto quit
%cc% ../cg_drawtools.c
@if errorlevel 1 goto quit
%cc% ../cg_effects.c
@if errorlevel 1 goto quit
%cc% ../cg_ents.c
@if errorlevel 1 goto quit
%cc% ../cg_event.c
@if errorlevel 1 goto quit
%cc% ../cg_info.c
@if errorlevel 1 goto quit
%cc% ../cg_localents.c
@if errorlevel 1 goto quit
%cc% ../cgx_extensions.c
@if errorlevel 1 goto quit
%cc% ../cg_main.c
@if errorlevel 1 goto quit
%cc% ../cg_marks.c
@if errorlevel 1 goto quit
%cc% ../cg_players.c
@if errorlevel 1 goto quit
%cc% ../cg_playerstate.c
@if errorlevel 1 goto quit
%cc% ../cg_predict.c
@if errorlevel 1 goto quit
%cc% ../cg_scoreboard.c
@if errorlevel 1 goto quit
%cc% ../cg_scoreboard_nemesis.c
@if errorlevel 1 goto quit
%cc% ../cg_servercmds.c
@if errorlevel 1 goto quit
%cc% ../cg_snapshot.c
@if errorlevel 1 goto quit
%cc% ../cg_view.c
@if errorlevel 1 goto quit
%cc% ../cg_weapons.c
@if errorlevel 1 goto quit
%cc% ../cg_unlagged.c
@if errorlevel 1 goto quit
%cc% ../cg_freeze.c
@if errorlevel 1 goto quit
%cc% ../cg_zeffects.c
@if errorlevel 1 goto quit

q3asm -f ../cgame

cd ../../baseq3
mkdir vm
cd ..
move cgame\vm\cgame.qvm baseq3\vm\cgame.qvm

:quit
cd ..
