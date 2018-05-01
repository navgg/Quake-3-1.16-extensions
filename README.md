# Quake III X-Mod 1.16n
Custom extensions for patching Quake 3 1.16n (beta version)

###### Installation & Requirements
- requires quake 3 arena version 1.16n
- to install unzip {X-Mod}.pk3 to baseq3 folder of your quake 3 arena installation path

###### Features
- wide screen fix, supports any ratio, icons not stretching, fov fix for widescreens
- extended ingame ui with short description of many settings
- added advanced settings menu with many hidden game settings
- extended display menu, brightness options, fps selection, primitives etc.
- extended network menu, rate options, added packets, snaps etc. (automatic or manual setting options)
- misc controls added to menu, like kill, screenshot etc.
- colored crossairs, 36 colors in total
- bright cpm skins for enemies and team with custom coloring
- cpm hitsounds based on damage dealt
- draw gun with no bobbing like in cpm
- weapon auto swith after respawn settings
- speedometer options, extended lagometer + ping or display only when packetloss
- display played id on scoreboard, useful for private chatting/following eg. \tell id \follow id
- ping colors, below 50 white, below 100 green, below 250 yellow, below 400 magenta, more than 400 red
- ability to disable chat beep or enemy taunt sounds
- resolving favorite servers by domain name
- integrated unlagged 2.01 (`client compatible with noghost\nemesis\bma servers`)
- optimization: removed some debug info
- shared q3config saving

###### Buf fixes
- some big maps loading fixed
- rewards display fixed if it's more than 10
- cinematics playback fixed for widescreens (only from cinematics menu and singleplayer games)
- colored server names shifting left bug fixed in server browser menu
- sarge/default bug model in team games
- display cg_shadow 1 when cg_marks 0
- scroll in serverinfo\driverinfo
- couldn't load map error messages
- fixed empty attacker icon when cg_draw3dicons 0

###### Authors
- X-Mod - (c) 2018 NaViGaToR (322)
- Unlagged 2.01 - (c) 2006 Neil “haste” Toronto
- CPMA - (c) 2000-2010 Challenge World, (c) 2016-2018 The ProMode Team
- Quake 3 Arena - (c) 1999-2005 Id Software
