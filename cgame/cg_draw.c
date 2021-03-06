// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

// set in CG_ParseTeamInfo
int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;
int drawTeamOverlayModificationCount = -1;

/*
==============
CG_DrawFieldEx
x-mod: extended method, negative width - align left
Draws large numbers for status bar and powerups
==============
*/
void CG_DrawFieldEx (int x, int y, int width, int value, float *rgba, int charWidth, int charHeight) {
	char	num[16], *ptr;
	int		l;
	int		frame;
	qboolean lalign = qfalse;

	if ( width < 0 ) {
		width = -width;
		lalign = qtrue;
	}

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	if (!lalign)
		 x += charWidth * (width - l);
	trap_R_SetColor( rgba );
	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x,y, charWidth, charHeight, cgs.media.numberShaders[frame] );
		x += charWidth;
		ptr++;
		l--;
	}
}

/*
==============
CG_DrawField
x-mod: vanilla like draw field
Draws large numbers for status bar and powerups
==============
*/
static void CG_DrawField(int x, int y, int width, int value, float *rgba) {
	x += 2;
	CG_DrawFieldEx(x, y, width, value, rgba, hud.char_width, hud.icon_size);
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModelColor(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles, byte *color, qhandle_t shader) {
	refdef_t		refdef;
	refEntity_t		ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer) {
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows
	ent.customShader = shader;

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;	

	ShaderRGBACopy(color, ent.shaderRGBA);

	trap_R_ClearScene();
	CGX_AddRefEntityWithCustomShader(&ent, 0);
	trap_R_RenderScene(&refdef);
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;	
	
	if ( !cg_draw3dIcons.integer || (!cg_drawIcons.integer && model != cgs.media.redFlagModel && model != cgs.media.blueFlagModel)) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;
	
	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles ) {
	clipHandle_t	cm;
	clientInfo_t	*ci;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs;

	ci = &cgs.clientinfo[ clientNum ];	

	if ( cg_draw3dIcons.integer ) {
		cm = ci->headModel;
		if ( !cm ) {
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5f * ( mins[2] + maxs[2] );
		origin[1] = 0.5f * ( mins[1] + maxs[1] );

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268f;	// len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd( origin, ci->headOffset, origin );
		CG_Draw3DModelColor(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles, ci->colors[1], ci->customShader);
	} else if ( cg_drawIcons.integer ) {		
		cm = ci->headModel;
		if ( !cm ) {			
			return;			
		}

		CG_DrawPic(x, y, w, h, ci->modelIcon);
	}

	// if they are deferred, draw a cross out
	if ( ci->deferred ) {
		CG_DrawPic( x, y, w, h, cgs.media.deferShader );
	} 
#if CGX_FREEZE
	else if ( Q_Isfreeze(clientNum)/* && cgx_scoreboard.integer != 3 */) {
		CG_DrawPic(x, y, w, h, cgs.media.noammoShader);
	}
#endif//freeze

}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel( float x, float y, float w, float h, int team ) {
	qhandle_t		cm;
	float			len;
	vec3_t			origin, angles;
	vec3_t			mins, maxs;

	if ( cg_draw3dIcons.integer ) {

		VectorClear( angles );

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5f * ( mins[2] + maxs[2] );
		origin[1] = 0.5f * ( mins[1] + maxs[1] );

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin( cg.time / 2000.0f );;

		CG_Draw3DModel( x, y, w, h, 
			team == TEAM_RED ? cgs.media.redFlagModel : cgs.media.blueFlagModel, 
			0, origin, angles );
	} else /*if ( cg_drawIcons.integer )*/ {
		gitem_t *item = BG_FindItemForPowerup( team == TEAM_RED ? PW_REDFLAG : PW_BLUEFLAG );

		CG_DrawPic( x, y, w, h, cg_items[ ITEM_INDEX(item) ].icon );
	}
}

/*
================
CG_DrawStatusBarHead

================
*/
void CG_DrawStatusBarHead( float x, float y, float w, float h ) {
	vec3_t		angles;
	float		size, stretch;
	float		frac;
	float		size_y;

	VectorClear( angles );

	if (cg_draw3dIcons.integer) {

		if (cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME) {
			frac = (float)(cg.time - cg.damageTime) / DAMAGE_TIME;
			size = w * (1.5f - frac * 0.5f);

			stretch = size - w;
			// kick in the direction of damage
			x -= stretch * 0.5f + cg.damageX * stretch * 0.5f;

			cg.headStartYaw = 180 + cg.damageX * 45;

			cg.headEndYaw = 180 + 20 * cos(crandom()*M_PI);
			cg.headEndPitch = 5 * cos(crandom()*M_PI);

			cg.headStartTime = cg.time;
			cg.headEndTime = cg.time + 100 + random() * 2000;
		} else {
			if (cg.time >= cg.headEndTime) {
				// select a new head angle
				cg.headStartYaw = cg.headEndYaw;
				cg.headStartPitch = cg.headEndPitch;
				cg.headStartTime = cg.headEndTime;
				cg.headEndTime = cg.time + 100 + random() * 2000;

				cg.headEndYaw = 180 + 20 * cos(crandom()*M_PI);
				cg.headEndPitch = 5 * cos(crandom()*M_PI);
			}

			size = w;
		}

		// if the server was frozen for a while we may have a bad head start time
		if (cg.headStartTime > cg.time) {
			cg.headStartTime = cg.time;
		}

		frac = (cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
		frac = frac * frac * (3 - 2 * frac);
		angles[YAW] = cg.headStartYaw + (cg.headEndYaw - cg.headStartYaw) * frac;
		angles[PITCH] = cg.headStartPitch + (cg.headEndPitch - cg.headStartPitch) * frac;

	} else {
		size = w;
	}

	size_y = h * size / w;

	//CG_DrawHead( x, SCREEN_HEIGHT - size, size, size, 
	//			cg.snap->ps.clientNum, angles );
	CG_DrawHead( x, y + h - size_y, size, size_y, 
			cg.snap->ps.clientNum, angles );
}

/*
================
CG_DrawStatusBarFlag

================
*/
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, hud.sby, hud.icon_size, hud.icon_size, team );
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team )
{
	vec4_t		hcolor;

	hcolor[3] = alpha;
	if ( team == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return; // no team
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );
}

/*
================
CG_DrawStatusBar

================
*/
static void CG_DrawStatusBar( void ) {
	int			color;
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	float		*hcolor;
	vec3_t		angles;
	vec3_t		origin;

	static float colors[4][4] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1, 0.69, 0, 1.0 } ,		// normal
		{ 1.0, 0.2, 0.2, 1.0 },		// low health
		{0.5, 0.5, 0.5, 1},			// weapon firing
		{ 1, 1, 1, 1 } };			// health > 100

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	if (hud.file) {
		if (cg.predictedPlayerState.powerups[PW_REDFLAG])
			CGX_DrawFlagModel(XH_StatusBar_Flag, TEAM_RED);
		else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
			CGX_DrawFlagModel(XH_StatusBar_Flag, TEAM_BLUE);
	} else {
		// draw the team background
		CG_DrawTeamBackground( 0, hud.sbteambg_y, vScreen.width, hud.head_size, 0.33f, ps->persistant[PERS_TEAM] );

		CG_DrawStatusBarHead( hud.sbheadx, hud.sbheady, hud.head_size, hud.head_size );

		if (cg.predictedPlayerState.powerups[PW_REDFLAG])
			CG_DrawStatusBarFlag( hud.sbflagx, TEAM_RED);
		else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
			CG_DrawStatusBarFlag( hud.sbflagx, TEAM_BLUE);
	}

	//
	// ammo
	//
	if ( cent->currentState.weapon ) {
		value = ps->ammo[cent->currentState.weapon];

		if ((unsigned int)value > 999 && cent->currentState.weapon != WP_GAUNTLET)
			value = 999;

		if ( value > -1 ) {
			if ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING
				&& cg.predictedPlayerState.weaponTime > 100 ) {
				// draw as dark grey when reloading
				color = 2;	// dark grey
			} else {
				if ( value >= 0 ) {
					color = 0;	// green
				} else {
					color = 1;	// red
				}
			}

			//trap_R_SetColor( colors[color] );
			if (hud.file) {
				if (xhud[XH_StatusBar_AmmoBar].inuse) {
					static int maxValues[WP_NUM_WEAPONS] = { -1, -1, 200, 50, 50, 50, 200, 50, 200, 50, 200 };
					int maxValue = maxValues[cent->currentState.weapon];

					CGX_DrawBar(XH_StatusBar_AmmoBar, value, maxValue, colors[0]);
				}
				CGX_DrawField(XH_StatusBar_AmmoCount, value, colors[color]);
			} else {
				CG_DrawField (hud.sbammo_tx, hud.sby, 3, value, colors[color]);
			}

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if ( cg_drawIcons.integer ) {
				if ( !cg_draw3dIcons.integer && !(xhud[XH_StatusBar_AmmoIcon].flags & XF_DRAW3D)) {
					qhandle_t icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;
					if (icon) {
						if (hud.file)
							CGX_DrawPic(XH_StatusBar_AmmoIcon, icon);
						else
							CG_DrawPic(hud.sbammox, hud.sby, hud.icon_size, hud.icon_size, icon);
					}
				} else if ( cg_weapons[ cent->currentState.weapon ].ammoModel ) {
					VectorClear( angles );

					origin[0] = 70;
					origin[1] = 0;
					origin[2] = 0;
					if (hud.file) {
						CGX_Draw3DModel(XH_StatusBar_AmmoIcon, cg_weapons[cent->currentState.weapon].ammoModel, 0, origin, angles);
					} else {
						angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );
						CG_Draw3DModel( hud.sbammox, hud.sby, hud.icon_size, hud.icon_size,
							cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles );
					}
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if ( value > 100 ) {
		hcolor = colors[3];
	} else if (value > 25) {
		hcolor = colors[0];
	} else if (value > 0) {
		color = (cg.time >> 8) & 1;	// flash
		hcolor = colors[color];
	} else {
		hcolor = colors[1];
	}

	// stretch the health up when taking damage
	if (hud.file) {
		//ps->stats[STAT_MAX_HEALTH] * 2
		CGX_DrawBar(XH_StatusBar_HealthBar, value, 200, hcolor);
		CGX_DrawField(XH_StatusBar_HealthCount, value, hcolor);
		CGX_DrawPic(XH_StatusBar_HealthIcon, 0);
	} else {
		CG_DrawField ( hud.sbhealth_tx, hud.sby, 3, value, hcolor );
	}

	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0 ) {
		//trap_R_SetColor( colors[0] );
		if (hud.file) {
			//hcolor = (value > 100) ? colors[3] : colors[0];
			CGX_DrawBar(XH_StatusBar_ArmorBar, value, 200, colors[0]);
			CGX_DrawField(XH_StatusBar_ArmorCount, value, colors[0]);
		} else {
			CG_DrawField ( hud.sbarmor_tx, hud.sby, 3, value, colors[0] );
		}
		
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if (cg_drawIcons.integer) {
			if (!cg_draw3dIcons.integer && !(xhud[XH_StatusBar_ArmorIcon].flags & XF_DRAW3D)) {
				if (hud.file)
					CGX_DrawPic(XH_StatusBar_ArmorIcon, cgs.media.armorIcon);
				else
					CG_DrawPic(hud.sbarmorx, hud.sby, hud.icon_size, hud.icon_size, cgs.media.armorIcon);
			} else {
				origin[0] = 90;
				origin[1] = 0;
				origin[2] = -10;
				if (hud.file) {
					CGX_Draw3DModel(XH_StatusBar_ArmorIcon, cgs.media.armorModel, 0, origin, angles);
				} else {
					angles[YAW] = (cg.time & 2047) * 360 / 2048.0f;
					CG_Draw3DModel(hud.sbarmorx, hud.sby, hud.icon_size, hud.icon_size,
						cgs.media.armorModel, 0, origin, angles);
				}
			}
		}
	}

	trap_R_SetColor( NULL );
}

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
static float CG_DrawAttacker( float y ) {
	int			t;
	float		size;
	vec3_t		angles;
	const char	*info;
	const char	*name;
	int			clientNum;

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	if ( !cg.attackerTime ) {
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return y;
	}

	t = cg.time - cg.attackerTime;
	if ( t > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return y;
	}

	info = CG_ConfigString( CS_PLAYERS + clientNum );
	name = Info_ValueForKey(  info, "n" );

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;

	if (hud.file /*xhud[XH_AttackerName].inuse || xhud[XH_AttackerIcon].inuse*/) {
		CGX_DrawHead(XH_AttackerIcon, clientNum, angles);
		CGX_DrawString(XH_AttackerName, va("^7%s", name), 0);
		return 0;
	} else {

		size = hud.head_size;
		trap_R_SetColor(NULL);
		CG_DrawHead(vScreen.width - size, y, size, size, clientNum, angles);
		y += size;
		CG_DrawBigString2(vScreen.width - (Q_PrintStrlen(name) * hud.big_char_w), y, name, 0.5);

		return y + BIGCHAR_HEIGHT + 2;
	}
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( hud.width5 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );

		if (hud.file) {
			CGX_DrawString( XH_FPS, s, 0);
			return 0;
		}

		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

		CG_DrawBigString( hud.width5 - w, y + 2, s, 1.0F);
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char		*s;
	int			w;
	int			mins, seconds, tens;
	int			msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );

	if (hud.file/*xhud[XH_GameTime].inuse*/) {
		CGX_DrawString(XH_GameTime, s, 0);
		return 0;
	}
	
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	if (cg_drawTimer.integer == 1) {
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);

		return y + BIGCHAR_HEIGHT + 4;
	} else {
		CG_DrawBigString(vScreen.hwidth - w / 2, 0 + 2, s, 1.0f);

		return y;
	}
}

/*
================
CGX_DrawSpeedMeter
================
based on code baseq3a: https://github.com/ec-/baseq3a
*/
static float CGX_DrawSpeedMeter(float y) {
	char	*s;
	int		w;

	if (hud.file/*xhud[XH_PlayerSpeed].inuse*/ && cgx_drawSpeed.integer != 2) {
		CGX_DrawString(XH_PlayerSpeed, va("%1.0fups", cg.xyspeed), 0);
		return 0;//y + BIGCHAR_HEIGHT + 4;
	}

	s = va((cg.xyspeed >= 500 ? "^3%1.0fups" : "%1.0fups"), cg.xyspeed);

	if (cgx_drawSpeed.integer == 1) {
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

		/* top left-hand corner of screen */
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0f);
		return y + BIGCHAR_HEIGHT + 4;
	}
	else {
		w = CG_DrawStrlen(s) * hud.big_char_w;

		/* center of screen */
		if (!cg.scoreBoardShowing)
			CG_DrawBigString2(vScreen.hwidth - w / 2, 243 + cg_crosshairSize.integer / 2, s, 0.5f);
		return y;
	}
}

/*
=================
CGX_DrawAcc
=================
*/
static float CGX_DrawAcc( float y ) {
	char *s;
	int i, w;
	
	i = cg.snap->ps.persistant[PERS_ACCURACY_SHOTS];
	if (i > 0)
		i = cg.snap->ps.persistant[PERS_ACCURACY_HITS] * 1000 / i;
	else
		i = 0;	

	w = i / 10;

	s = va("Acc: %i.%i", w, i - w * 10);

	if (hud.file) {
		CGX_DrawString(XH_PlayerAccuracy, s, 0);
		return 0;
	}
	
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);	

	return y + BIGCHAR_HEIGHT + 4;	
}
#if CGX_DEBUG
static float CG_DrawFreeMem(float y) {
	char *s;
	int  w;

	s = va("Mem: %i Mb", trap_MemoryRemaining() / 1024 / 1024);
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;	

	CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);	

	return y + BIGCHAR_HEIGHT + 4;	
}
#endif

#if 0
/*
=================
CGX_DrawAngles
=================
*/
static float CGX_DrawAngles( float y ) {
	char *s;	
	int w;
	vec3_t angles;

	VectorCopy(cg.snap->ps.viewangles, angles);	

	s = va("%f'", angles[1]);	
	w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;

	if (1) {
		CG_DrawSmallString(vScreen.hwidth - w / 2 + 32, 240 - SMALLCHAR_HEIGHT / 2, s, 0.5f);
		return y;
	}	

	CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);	

	return y + BIGCHAR_HEIGHT + 4;	
}

/*
=================
CGX_DrawButtons
=================
*/
//static float CGX_DrawButtons( float y ) {
//	char *s;	
//	int w, i;	
//	char fm, rm, um;		
//
//	cg.snap->ps.pm_flags
//
//	if (cg_pmove.cmd.forwardmove)
//		fm = cg_pmove.cmd.forwardmove > 0 ? 'W': 'S';
//	else 
//		fm = ' ';	
//
//	if (cg_pmove.cmd.rightmove)
//		rm = cg_pmove.cmd.rightmove > 0 ? '>': '<';
//	else 
//		rm = ' ';	
//
//	if (cg_pmove.cmd.upmove)
//		um = cg_pmove.cmd.upmove > 0 ? 'J': 'C';
//	else 
//		um = ' ';	
//
//	s = va("%c %c %c %c", 		
//		fm, 		
//		rm, 		
//		um,
//		cg_pmove.cmd.buttons & BUTTON_ATTACK);
//
//	w = CG_DrawStrlen( s ) * SMALLCHAR_WIDTH;
//
//	if (1) {
//		CG_DrawSmallString(vScreen.hwidth - w / 2 - 64, 240 - SMALLCHAR_HEIGHT / 2, s, 0.5f);
//		return y;
//	}	
//
//	CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);	
//
//	return y + BIGCHAR_HEIGHT + 4;	
//}
#endif

/*
=================
CG_DrawTeamOverlay
=================
*/

//#define TEAM_OVERLAY_MAXNAME_WIDTH	12
//#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

static float CG_DrawTeamOverlay( float y, qboolean right, qboolean upper ) {
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	int ret_y;

	if ( !cg_drawTeamOverlay.integer ) {
		return y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED &&
		cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE )
		return y; // Not on any team

	plyrs = 0;

	// max player name width
	pwidth = 0;
	for (i = 0; i < numSortedTeamPlayers; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	if (hud.file) {
		CGX_DrawTeamOverlay(pwidth, lwidth);
		return y;
	}

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if ( right )
		x = vScreen.width - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if ( upper ) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
		hcolor[3] = 0.33;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
		hcolor[3] = 0.33;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );

	for (i = 0; i < numSortedTeamPlayers; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y,
				ci->name, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
				//len = CG_DrawStrlen(p);
				//if (len > lwidth)
				//	len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt( xx, y,
					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth( ci->health, ci->armor, hcolor );

			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 + 
				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt( xx, y,
				st, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
					cg_weapons[ci->curWeapon].weaponIcon );
			} else {
				CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
					cgs.media.deferShader );
			}

			// Draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j < PW_NUM_POWERUPS; j++) {
#if CGX_FREEZE//freeze
				if ( Q_Isfreeze( ci - cgs.clientinfo ) ) {
					CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.noammoShader );
					break;
				}
#endif//freeze
				if (ci->powerups & (1 << j)) {
					gitem_t	*item;

					item = BG_FindItemForPowerup( j );

					CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
						trap_R_RegisterShader( item->icon ) );
					if (right) {
						xx -= TINYCHAR_WIDTH;
					} else {
						xx += TINYCHAR_WIDTH;
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
}
#if CGX_DEBUG
static float CGX_DrawDebugInfo(float y);
#endif
/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( void ) {
	float	y;

	y = 0;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qtrue );
	} 
#if CGX_DEBUG
	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
#endif
	if ( cg_drawFPS.integer ) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimer.integer ) {
		y = CG_DrawTimer( y );
	}
	if (cgx_drawSpeed.integer) {
		y = CGX_DrawSpeedMeter(y);
	}
	if (cgx_drawAccuracy.integer) {
		y = CGX_DrawAcc(y);
	}

	if (cgs.countAlive)
		y = CGX_DrawTeamCounts(y);

#if 0
	// angles still not checked in other defrags or quake versions
	y = CGX_DrawAngles(y);
	// buttons only can draw of current client
	y = CGX_DrawButtons(y);
#endif

#if CGX_DEBUG
	if (cgx_debug.integer) {
		y = CGX_DrawDebugInfo(y);

		y = CG_DrawFreeMem(y);
	}
#endif
	if ( cg_drawAttacker.integer ) {
		y = CG_DrawAttacker( y );
	}
}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
static float CG_DrawScores( float y ) {
	const char	*s;
	int			s1, s2, score;
	int			x, w;
	int			v;
	vec4_t		color;
	float		y1;
	gitem_t		*item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;
	
	y -= hud.score_yofs;
	
	y1 = y;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		int team = cg.snap->ps.persistant[PERS_TEAM];
		qboolean spectator = team == TEAM_SPECTATOR;

		x = vScreen.width;

		if (hud.file) {
			if (spectator) {
				CGX_DrawString(XH_Score_NME, va("%i", s2), 0);
			} else {
				CGX_DrawString(team == TEAM_BLUE ? XH_Score_OWN : XH_Score_NME, va("%i", s2), 0);
			}
		} else {
			color[0] = 0;
			color[1] = 0;
			color[2] = 1;
			color[3] = 0.33;
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			if (team == TEAM_BLUE) {
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			}
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_BLUEFLAG );

			if (item) {
				if (hud.file) {
					if (spectator) {
						CGX_DrawFlagStatus(XH_FlagStatus_NME, PW_BLUEFLAG);
					} else {
						CGX_DrawFlagStatus(team == TEAM_BLUE ? XH_FlagStatus_OWN : XH_FlagStatus_NME, PW_BLUEFLAG);
					}
				} else {
					y1 = y - BIGCHAR_HEIGHT - 8;
					if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
						CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag] );
					}
				}
#if 0
				CG_RegisterItemVisuals( ITEM_INDEX(item) );
				switch (cgs.blueflag) {
				case 0 :  // at base
					// Draw the icon
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, trap_R_RegisterShader( item->icon ) );
				case 1 : // taken
					CG_FillRect( x, y1-4,  w, BIGCHAR_HEIGHT+8, color );
					break;
				case 2 : // droppped, grey background
					color[0] = color[1] = color[2] = 1;
					CG_FillRect( x, y1-4,  w, BIGCHAR_HEIGHT+8, color );
					break;
				// Other values won't draw (-1 is disabled)
				}
#endif
			}
		}

		if (hud.file) {
			if (spectator) {
				CGX_DrawString(XH_Score_OWN, va( "%i", s1 ), 0);
			} else {
				CGX_DrawString(team == TEAM_RED ? XH_Score_OWN : XH_Score_NME, va( "%i", s1 ), 0);
			}
		} else {
			color[0] = 1;
			color[1] = 0;
			color[2] = 0;
			color[3] = 0.33;
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			if (team == TEAM_RED) {
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			}
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( PW_REDFLAG );

			if (item) {
				if (hud.file) {
					if (spectator) {
						CGX_DrawFlagStatus(XH_FlagStatus_OWN, PW_REDFLAG);
					} else {
						CGX_DrawFlagStatus(team == TEAM_RED ? XH_FlagStatus_OWN : XH_FlagStatus_NME, PW_REDFLAG);
					}
				} else {
					y1 = y - BIGCHAR_HEIGHT - 8;
					if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
						CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag] );
					}
				}
#if 0
				CG_RegisterItemVisuals( ITEM_INDEX(item) );
				switch (cgs.redflag) {
				case 0 :  // at base
					// Draw the icon
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, trap_R_RegisterShader( item->icon ) );
					break;
				case 1 : // taken
					CG_FillRect( x, y1-4,  w, BIGCHAR_HEIGHT+8, color );
					break;
				case 2 : // droppped, grey background
					color[0] = color[1] = color[2] = 1;
					CG_FillRect( x, y1-4,  w, BIGCHAR_HEIGHT+8, color );
					break;
				// Other values won't draw (-1 is disabled)
				}
#endif
			}
		}


		if ( cgs.gametype == GT_CTF ) {
			v = cgs.capturelimit;
		} else {
			v = cgs.fraglimit;
		}
		if ( v ) {
			if (hud.file) {
				CGX_DrawString(XH_Score_Limit, va( "%i", v ), 0);
			} else {
				s = va( "%2i", v );
				w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
				x -= w;
				CG_DrawBigString( x + 4, y, s, 1.0F);
			}
		}

	} else {
		qboolean	spectator;

		x = vScreen.width;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		if (hud.file) {
			if (!spectator) {
				int scoreNME = score == s1 ? s2 : s1;

				CGX_DrawString(XH_Score_OWN, va("%i", score), 0);
				if (scoreNME != SCORE_NOT_PRESENT)
					CGX_DrawString(XH_Score_NME, va("%i", scoreNME), 0);
			} else {
				if (s1 != SCORE_NOT_PRESENT)
					CGX_DrawString(XH_Score_OWN, va("%i", s1), 0);
				if (s2 != SCORE_NOT_PRESENT)
					CGX_DrawString(XH_Score_NME, va("%i", s2), 0);
			}
		} else {
			// always show your score in the second box if not in first place
			if ( s1 != score ) {
				s2 = score;
			}
			if ( s2 != SCORE_NOT_PRESENT ) {
				s = va( "%2i", s2 );
				w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
				x -= w;
				if ( !spectator && score == s2 && score != s1 ) {
					color[0] = 1;
					color[1] = 0;
					color[2] = 0;
					color[3] = 0.33;
					CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
					CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
				} else {
					color[0] = 0.5;
					color[1] = 0.5;
					color[2] = 0.5;
					color[3] = 0.33;
					CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				}	
				CG_DrawBigString( x + 4, y, s, 1.0F);
			}

			// first place
			if ( s1 != SCORE_NOT_PRESENT ) {
				s = va( "%2i", s1 );
				w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
				x -= w;
				if ( !spectator && score == s1 ) {
					color[0] = 0;
					color[1] = 0;
					color[2] = 1;
					color[3] = 0.33;
					CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
					CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
				} else {
					color[0] = 0.5;
					color[1] = 0.5;
					color[2] = 0.5;
					color[3] = 0.33;
					CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				}
				CG_DrawBigString( x + 4, y, s, 1.0F);
			}
		}

		if ( cgs.fraglimit ) {
			if (hud.file) {
				CGX_DrawString(XH_Score_Limit, va("%i", cgs.fraglimit), 0);
			} else {
				s = va( "%2i", cgs.fraglimit );
				w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
				x -= w;
				CG_DrawBigString( x + 4, y, s, 1.0F);
			}
		}

	}

	return y1 - 8;
}

/*
================
CG_DrawPowerups
================
*/
static float CG_DrawPowerups( float y ) {
	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	int		i, j, k;
	int		active;
	playerState_t	*ps;
	int		t;
	gitem_t	*item;
	int		x;
	int		color;
	float	size;
	float	f;
	static float colors[2][4] = { 
		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 } };

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( !ps->powerups[ i ] ) {
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		//if xhud used and no StatusBar_Flag element then set flag time to 0, PowerUp1_Icon will be used for flag
		if (hud.powerupsMaxNum && (i == PW_REDFLAG || i == PW_BLUEFLAG) && !xhud[XH_StatusBar_Flag].inuse)
			t = 0;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if ( t < 0 || t > 999000) {
			continue;
		}

		// insert into the list
		for ( j = 0 ; j < active ; j++ ) {
			if ( sortedTime[j] >= t ) {
				for ( k = active - 1 ; k >= j ; k-- ) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	if (hud.powerupsMaxNum) {
		//draw powerups
		for ( i = 0 ; i < active && i < hud.powerupsMaxNum ; i++ ) {
			t = sortedTime[i] / 1000;
			item = BG_FindItemForPowerup( sorted[i] );

			if (sorted[i] == PW_BLUEFLAG) {
				CGX_DrawFlagModel(XH_PowerUp1_Icon + i, TEAM_BLUE);
			} else if (sorted[i] == PW_REDFLAG) {
				CGX_DrawFlagModel(XH_PowerUp1_Icon + i, TEAM_RED);
			} else {
				CGX_DrawString(XH_PowerUp1_Time + i, va("%i", t), 0);
				CGX_DrawPic(XH_PowerUp1_Icon + i, trap_R_RegisterShader(item->icon));
			}
		}

		//draw holdable in powerup list
		if (i < hud.powerupsMaxNum) {
			int value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
			if (value) {
				CG_RegisterItemVisuals(value);
				CGX_DrawPic(XH_PowerUp1_Icon + i, cg_items[value].icon);
			}
		}

		return y;
	}

	// draw the icons and timers
	x = vScreen.width - hud.icon_size - hud.char_width * 2;
	for ( i = 0 ; i < active ; i++ ) {
		item = BG_FindItemForPowerup( sorted[i] );

		color = 1;

		y -= hud.icon_size;

		//trap_R_SetColor( colors[color] );
		CG_DrawField( x, y, 2, sortedTime[ i ] / 1000, colors[color] );

		t = ps->powerups[ sorted[i] ];
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			trap_R_SetColor( NULL );
		} else {
			vec4_t	modulate;

			f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
			f -= (int)f;
			modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
			trap_R_SetColor( modulate );
		}

		if ( cg.powerupActive == sorted[i] && 
			cg.time - cg.powerupTime < PULSE_TIME ) {
			f = 1.0 - ( ( (float)cg.time - cg.powerupTime ) / PULSE_TIME );
			size = hud.icon_size * ( 1.0 + ( PULSE_SCALE - 1.0 ) * f );
		} else {
			size = hud.icon_size;
		}

		CG_DrawPic( vScreen.width - size, y + (hud.icon_size - size) / 2, 
			size, size, trap_R_RegisterShader( item->icon ) );
	}
	trap_R_SetColor( NULL );

	return y;
}


/*
=====================
CG_DrawLowerRight

=====================
*/
static void CG_DrawLowerRight( void ) {
	float	y;

	y = hud.sby;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qfalse );
	} else if (!cg_lagometer.integer && hud.type != HUD_VANILLAQ3 && (vScreen.offsetx > 16 || !cg_drawIcons.integer)) {
		y += hud.score_yofs_no_lagometer;
	}

	if (cgx_drawScoreBox.integer) {
		y = CG_DrawScores(y);
	}

	y = CG_DrawPowerups( y );
}

/*
===================
CG_DrawPickupItem
===================
*/
static int CG_DrawPickupItem( int y ) {
	int		value;
	float	*fadeColor;

	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	y -= hud.icon_size;

	value = cg.itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.itemPickupTime, hud.pickupTime );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );
			trap_R_SetColor( fadeColor );

			if (hud.file) {
				CGX_DrawPic( XH_ItemPickupIcon, cg_items[ value ].icon );
				CGX_DrawString( XH_ItemPickup, bg_itemlist[value].pickup_name, fadeColor[0] );
			} else {
				CG_DrawPic( 8, y, hud.icon_size, hud.icon_size, cg_items[ value ].icon );
				CG_DrawBigString2( hud.icon_size + hud.big_char_w, y + (hud.icon_size - hud.big_char_w) / 2, bg_itemlist[ value ].pickup_name, fadeColor[0] );
			}
			trap_R_SetColor( NULL );
		}
	}
	
	return y;
}

/*
=====================
CG_DrawLowerLeft

=====================
*/
static void CG_DrawLowerLeft( void ) {
	float	y;

	y = hud.sby;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3 ) {
		y = CG_DrawTeamOverlay( y, qfalse, qfalse );
	} 

	y = CG_DrawPickupItem( y );
}



//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo( void ) {
	int w, h;
	int i, len;
	vec4_t		hcolor;
	int		chatHeight;

#define CHATLOC_Y hud.sbteambg_y // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++) {
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if (len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			hcolor[0] = 1;
			hcolor[1] = 0;
			hcolor[2] = 0;
			hcolor[3] = 0.33f;
		} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 1;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0;
			hcolor[1] = 1;
			hcolor[2] = 0;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor( hcolor );
		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, vScreen.width, h, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH, 
				CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT, 
				cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
		}
	}
}

/*
===================
CG_DrawHoldableItem
===================
*/
static void CG_DrawHoldableItem( void ) { 
	int		value;

	//if hud loaded from file, holdable will be drawn with powerups
	if (hud.powerupsMaxNum)
		return;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );
		CG_DrawPic( vScreen.width-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
	}

}


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) { 
	float	*color;
	int		i;
	float	x, y;

	if ( !cg_drawRewards.integer ) {
		return;
	}
	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if ( !color ) {
		return;
	}
	
	trap_R_SetColor(color);

	if ( cg_drawRewards.integer > 1 || cg.rewardCount > 10) {
		char buf[6];		
		y = 35; // 
		x = vScreen.hwidth - ICON_SIZE/2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader );
		Com_sprintf( buf, sizeof( buf ), "%d", cg.rewardCount );
		CG_DrawStringExt( 
			(vScreen.width - strlen( buf ) * SMALLCHAR_WIDTH) / 2 - 1, 
			y + ICON_SIZE, 
			buf, color, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
	} else {				
		y = 56;
		x = vScreen.hwidth - cg.rewardCount * ICON_SIZE / 2;
		for (i = 0; i < cg.rewardCount; i++) {
			CG_DrawPic(x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader);
			x += ICON_SIZE;
		}		
	}

	trap_R_SetColor(NULL);
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;

	// X-MOD: extend to store lost packets and delayed rate in LAG_SAMPLES interval;
	int		packetloss;
	int		rateDelayed;
} lagometer_t;

lagometer_t		lagometer;

// X-MOD: update network stats for lagometer and other purpose
static void CGX_UpdateNetworkStats(snapshot_t *snap) {
	static int pingTotal, pingCount, oldSnapshotCount;

	//skip in intermission and skip localserver
	if (cg.intermissionStarted || (cgs.localServer && !xhud[XH_NetGraphPing].inuse))
		return;	

	if (snap->ping != 999) {
		pingTotal += snap->ping;
		pingCount++;
	}		

	if (snap->snapFlags & SNAPFLAG_RATE_DELAYED)
		lagometer.rateDelayed++;				

	// still collecting stats
	if (oldSnapshotCount > lagometer.snapshotCount)
		return;

#if CGX_DEBUG
	if (cgx_debug.integer & 2)
		D_Printf(("CGX_UpdateNetworkStats %i %i %i\n", cg.meanPing, lagometer.packetloss, lagometer.rateDelayed));
#endif

	oldSnapshotCount = lagometer.snapshotCount + LAG_SAMPLES;

	cg.meanPing = pingCount ? pingTotal / pingCount : -1;		
	cg.packetloss = lagometer.packetloss;
	cg.rateDelayed = lagometer.rateDelayed;
	cg.packetlossTotal += lagometer.packetloss;
	cg.rateDelayedTotal += lagometer.rateDelayed;	

	//reset counters for next time interval

	pingTotal = 0;
	pingCount = 0;
	lagometer.packetloss = 0;
	lagometer.rateDelayed = 0;
}

// auto adjust fps after 250ms of "connection interrupted"
static void CGX_CheckFps() {
	if (cgx_networkAdjustments.integer) {
		static int checktime;
		int ping;
		checktime += cg.frametime;

		if (checktime < 250)
			return;

		ping = cg.nextSnap->ping;

		D_Printf(("CGX_CheckFps time %d ping %d\n", checktime, ping));

		checktime = 0;

		if (com_maxfps.integer > 125 && ping > 150 && ping != 999) {
			char buf[MAX_QPATH];

			trap_Cvar_VariableStringBuffer("cgx_com_maxfps", buf, sizeof(buf));
			if (!*buf)
				trap_Cvar_Set("cgx_com_maxfps", com_maxfps.string);
			
			if (com_maxfps.integer > 200 && ping <= 200) {
				trap_Cvar_Set("com_maxfps", "200");
				trap_Print("^3WARNING: Fps was set to 200 because of high ping\n");
			} else {
				trap_Cvar_Set("com_maxfps", "125");
				trap_Print("^3WARNING: Fps was set to 125 because of high ping\n");
			}
		}
	}
}

#if CGX_DEBUG
extern int cg_marksTotal;
// X-MOD: display custom debug info
static float CGX_DrawDebugInfo( float y ) {	
	char		*s;
	int			w;
	static int  t = -1, opes, ppes, ppms, opms, ppds, opds;
	centity_t *cent;

	if (cgx_debug.integer & 4) {
		s = va("ES: %i PS: %i MS: %i", cg.entities, cg.activeParticles, cg_marksTotal);
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( hud.width5 - w, y + 2, s, 1.0F);	
		y += BIGCHAR_HEIGHT + 4;

		cent = &cg_entities[cg.snap->ps.clientNum];

		s = va( "LO: %1.0f %1.0f %1.0f", cent->lerpOrigin[0], cent->lerpOrigin[1], cent->lerpOrigin[2] );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( hud.width5 - w, y + 2, s, 1.0F );
		y += BIGCHAR_HEIGHT + 4;
	}

	if (cg_showmiss.integer) {
		if (trap_Milliseconds() > t) {
			t = trap_Milliseconds() + 1000;
			ppes = cg.predictionErrors - opes;
			opes = cg.predictionErrors;
			ppms = cg.predictionMisses - opms;
			opms = cg.predictionMisses;
			ppds = cg.predictionDecays - opds;
			opds = cg.predictionDecays;
		}

		s = va("PE: %2i %2i", ppes, cg.predictionErrors - opes);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("PM: %2i %2i", ppms, cg.predictionMisses - opms);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("PD: %2i %2i", ppds, cg.predictionDecays - opds);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va( "OP: %2i %2i", cg.numPredicted, cg.numPlayedBack );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString( hud.width5 - w, y + 2, s, 1.0F );
		y += BIGCHAR_HEIGHT + 4;
	}

	if (cgx_debug.integer & 2) {
		s = va("CI: %i", cg.connectionInterrupteds);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("LSN: %i", cg.latestSnapshotNum);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("LSC: %i", lagometer.snapshotCount);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("PING: %i", cg.meanPing);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("LOSS: %i %i", cg.packetloss, cg.packetlossTotal);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("RD: %i %i", cg.rateDelayed, cg.rateDelayedTotal);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += BIGCHAR_HEIGHT + 4;

		s = va("T: %i", cg.time);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(hud.width5 - w, y + 2, s, 1.0F);
		y += +BIGCHAR_HEIGHT + 4;
	}

	return y;
}
#endif
/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		lagometer.packetloss++;	

		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;

	if (cg_lagometer.integer > 1 || xhud[XH_NetGraphPing].inuse/* || cgx_networkAdjustments.integer*/)
		CGX_UpdateNetworkStats(snap);		
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
		return;
#if CGX_DEBUG
	// X-MOD: count connection interrupteds	
	cg.connectionInterrupteds++;
#endif

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawBigString( vScreen.hwidth - w/2, 100, s, 1.0F);

	CGX_CheckFps();

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	if (hud.file) {
		CGX_DrawPic(XH_NetGraph, cgs.media.connectionShader);
		return;
	}

	x = hud.lagometer_x;
	y = hud.lagometer_y;

	CG_DrawPic(x, y, hud.icon_size, hud.icon_size, cgs.media.connectionShader );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || ( cgs.localServer && !xhud[XH_NetGraph].inuse ) ) {
		CG_DrawDisconnect();
		return;
	}

	// X-MOD: show lagometer only if lag, if no lag show disconnect icon
	if (cg_lagometer.integer > 2 && cg.rateDelayed == 0 && cg.packetloss == 0) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	if (hud.file) {
		CGX_DrawLagometer(XH_NetGraph, &ax, &ay, &aw, &ah, /*cgs.media.lagometerShader*/0);
	} else {
		ax = x = hud.lagometer_x;
		ay = y = hud.lagometer_y;
		aw = hud.icon_size;
		ah = hud.icon_size;
		trap_R_SetColor(NULL);
		CG_DrawPic(x, y, hud.icon_size, hud.icon_size, cgs.media.lagometerShader);
		CG_AdjustFrom640( &ax, &ay, &aw, &ah );
	}

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	if (cg_lagometer.integer == 1 && !hud.file) {
		if ( cg_nopredict.integer || cg_syncronousClients.integer )
			CG_DrawBigString2( x, y, "snc", 1.0 );
	} else {
		// X-MOD: draw ping in lagometer
		 if (!cg.demoPlayback) {
			char *pingStr;

			if (cg_nopredict.integer || cg_syncronousClients.integer) {
				pingStr = "snc";
			} else if (cg.meanPing >= 0) {
				// draw ping packet loss and delayed rate if lag
				if (cg_lagometer.integer > 2)
					pingStr = va("%ims %i %i", cg.meanPing, cg.packetloss, cg.rateDelayed);
				else
					pingStr = va("%ims", cg.meanPing);
			} else {
				// draw if stats not calculated yet
				pingStr = "...";
			}

			if (hud.file)
				CGX_DrawString(XH_NetGraphPing, pingStr, 0);
			else
				CG_DrawStringExt(x + 1, y, pingStr, colorWhite, qfalse, qfalse, hud.lagometer_fw, hud.lagometer_fh, 0);
		}
	}
	
	CG_DrawDisconnect();
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	if (hud.type == HUD_COMPACT)
		cg.centerPrintCharWidth = charWidth / 1.33333333f;
	else
		cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}

// X-MOD: cg_centerPrintAlpha
#define CGX_CheckCenterPrintAlpha(x) if (x > cgx_centerPrintAlpha.value) { x = cgx_centerPrintAlpha.value; }
/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		x, y, w;
	float	*color;

	if (hud.file && hud.fragTime) {
		if (color = CG_FadeColor(hud.fragTime, xhud[XH_FragMessage].time)) {
			CGX_CheckCenterPrintAlpha(color[3]);
			CGX_DrawString(XH_FragMessage, hud.fragMessage, color[3]);
		}
	}
	if (hud.file && hud.rankTime) {
		if (color = CG_FadeColor(hud.rankTime, xhud[XH_RankMessage].time)) {
			CGX_CheckCenterPrintAlpha(color[3]);
			CGX_DrawString(XH_RankMessage, hud.rankMessage, color[3]);
		}
	}

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );	
	if ( !color ) {
		return;
	}

	CGX_CheckCenterPrintAlpha(color[3]);

	trap_R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 40; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( vScreen.width - w ) / 2;

		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5f), 0 );

		y += cg.centerPrintCharWidth * 1.5f;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void) {
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;	

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if ( cg.renderingThirdPerson ) {
		return;
	}
	
	// X-MOD: set crosshair color
	if (cgx_crosshairColor.integer) {	
		trap_R_SetColor(g_color_table_ex[cgx_crosshairColor.integer % ArrLen(g_color_table_ex)]);
	} else if (*cgx_crosshairColor.string) {
		//try set from string
		char c = QX_StringToColor(cgx_crosshairColor.string);
		if (c) {
			cgx_crosshairColor.integer = ColorIndex(c);			
			trap_R_SetColor(g_color_table_ex[cgx_crosshairColor.integer % ArrLen(g_color_table_ex)]);
		}
	}
	// set color based on health
	else if ( cg_crosshairHealth.integer ) {
		vec4_t		hcolor;

		CG_ColorForHealth( hcolor );
		trap_R_SetColor( hcolor );
	} else {
		trap_R_SetColor( NULL );
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640( &x, &y, &w, &h );
	
	{
		int cn = cg_drawCrosshair.integer % NUM_CROSSHAIRS;
		if (!*cgx_crosshairColor.string) {
			if (!(hShader = cgs.media.defaultCrosshair[cn]))
				hShader = cgs.media.defaultCrosshair[cn] = trap_R_RegisterShader(va("gfx/2d/crosshair%c", 'a' + cn));
		} else {
			if (!(hShader = cgs.media.crosshairShader[cn]))
				hShader = cgs.media.crosshairShader[cn] = trap_R_RegisterShader(va("gfx/2d/xm_crosshair%c", 'a' + cn));
		}
	}

	trap_R_DrawStretchPic( x + cg.refdef.x + 0.5f * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5f * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader );	
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 8192, cg.refdef.viewaxis[0], end );

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );
	if ( trace.entityNum >= MAX_CLIENTS ) {
#if CGX_FREEZE
		entityState_t	*es;

		es = &cg_entities[ trace.entityNum ].currentState;
		if ( es->powerups & ( 1 << PW_QUAD ) || es->powerups & ( 1 << PW_BATTLESUIT ) ) { //noghost using quad, default freezetag battlesuit
			cg.crosshairClientNum = es->otherEntityNum;
			cg.crosshairClientTime = cg.time;
		}
#endif //freeze
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// if the player is invisible, don't show it
	if ( cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS ) ) {
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


//osp like bar draw
void CGX_DrawHPArmorBar( int startY, int value, int maxValue, float alpha ) {
	const int barHeight = 2;
	const int barWidth = 100;
	int index;
	int barValue = (value > maxValue) ? maxValue : value;
	int startX = vScreen.hwidth - barWidth / 2;
	vec4_t color, shadow;

	if (value > 100)
		index = 7;
	else if (value >= 75)
		index = 2;
	else if (value > 25)
		index = 3;
	else
		index = 1;

	Vector4Copy( g_color_table_ex[index], color );
	Vector4Copy( g_color_table_ex[0], shadow );

	color[3] *= alpha;
	shadow[3] *= alpha;

	CG_DrawWidthGauge( startX + 1,
		startY + 1,
		barWidth,
		barHeight,
		shadow,
		barValue, qfalse );

	CG_DrawWidthGauge( startX,
		startY,
		barWidth,
		barHeight,
		color,
		barValue, qfalse );
}

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float		*color;
	char		*name;
	float		w;
	clientInfo_t *ci;
	int			y0, i;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

	ci = &cgs.clientinfo[cg.crosshairClientNum];

	if (cg_drawCrosshairNames.integer > 0) {
		y0 = 170;
		i = cg_drawCrosshairNames.integer;
	} else {
		y0 = 310 - hud.big_char_w;
		i = -cg_drawCrosshairNames.integer;
	}

	if (i & 32)
		name = va("%s ^7%i", ci->name, cg.crosshairClientNum);
	else
		name = ci->name;

	color[3] *= 0.5f;

	if (hud.file) {
		CGX_DrawString(XH_TargetName, va("^7%s", name), color[3]);
	} else {
		w = CG_DrawStrlen(name) * hud.big_char_w;
		CG_DrawBigString2(vScreen.hwidth - w / 2, y0, name, color[3]);
	}

	if (i > 1 && ci->health > 0 && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
		clientInfo_t *pci = &cgs.clientinfo[cg.snap->ps.clientNum];

		if (pci->team == ci->team && (pci->team == TEAM_RED || pci->team == TEAM_BLUE)) {
			int startY = y0 + hud.big_char_w + 2;

			if (hud.file) {
				CGX_DrawString(XH_TargetStatus, va("%i / %i", ci->health, ci->armor), color[3]);
			} else 
			if (i == 2) {
				// draw as text
				char *s = va( "%i / %i", ci->health, ci->armor );
				w = CG_DrawStrlen( s ) * TINYCHAR_WIDTH;
				CG_DrawStringExt( vScreen.hwidth - w / 2, startY, s, color, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
			} else if (i == 3) {
				// draw as bars
				CGX_DrawHPArmorBar( startY, ci->health, 100, color[3] );
				CGX_DrawHPArmorBar( startY + 3, ci->armor, 100, color[3] );
			}
		}
	}

	trap_R_SetColor( NULL );
}




//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
	const char *specMsg = "SPECTATOR";

	if (cg.scoreBoardShowing)
		return;

	if (hud.file) {
		CGX_DrawString(XH_SpecMessage, specMsg, 0);
	} else {
		CG_DrawBigString(vScreen.hwidth - 9 * 8, 440, specMsg, 1.0F);
	}

#if CGX_FREEZE
	if ( cgs.gametype == GT_TOURNAMENT || Q_Isfreeze( cg.snap->ps.clientNum ) ) {
#else
	if ( cgs.gametype == GT_TOURNAMENT ) {
#endif//freeze
		char *s = "waiting to play";
		int w = CG_DrawStrlen(s) * hud.big_char_w;
		CG_DrawBigString2(vScreen.hwidth - w / 2, SCREEN_HEIGHT - hud.big_char_w - hud.weap_icon_sub, s, 1.0F);
	}
	else if ( cgs.gametype == GT_TEAM || cgs.gametype == GT_CTF ) {
		char *s = "use the TEAM menu to play";
		int w = CG_DrawStrlen(s) * hud.big_char_w;
		CG_DrawBigString2(vScreen.hwidth - w / 2, SCREEN_HEIGHT - hud.big_char_w - hud.weap_icon_sub, s, 1.0F);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
	char	*s;
	int		sec;

	if ( !cgs.voteTime ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("VOTE(%i):%s yes(F1):%i no(F2):%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );
	if (hud.file) {
		CGX_DrawString(XH_VoteMessageWorld, s, 0);
		return;
	}
	CG_DrawSmallString( 0, 58, s, 1.0F );
}

/*
=================
CG_DrawIntermission
=================
*/
extern vmCvar_t cgx_helpShowed;
static void CG_DrawIntermission( void ) {
	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
		CG_DrawCenterString();
		return;
	}

	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();

	trap_Cvar_Update(&cgx_intermissionStats);
	if (cgx_intermissionStats.integer) {
		CG_statsWindow(WFX_FADEIN);
	} else if (!cg.statsWindow && !(cgx_helpShowed.integer & 1) && cgs.clientinfo[cg.snap->ps.clientNum].team != TEAM_SPECTATOR) {
		char key[32];
		trap_Cvar_VariableStringBuffer("cgx_scores_key", key, sizeof key);
		CG_DrawStringExt(8, SCREEN_HEIGHT - 8 - 8, va("Press %s to see your stats", *key ? key : "+scores key"), 
			colorWhite, qfalse, qtrue, WINDOW_FONTWIDTH, WINDOW_FONTHEIGHT, 0);
		if (cgx_helpShowed.integer == 0)
			trap_Cvar_Set("cgx_help_showed", "1");
	}
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {
	float		x;
	const char	*name;

	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}

	if (cg.scoreBoardShowing)
		return qtrue;

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	if (hud.file) {
		CGX_DrawString(XH_FollowMessage, va("Following ^7%s", name), 0);
		return qtrue;
	}

	CG_DrawBigString( vScreen.hwidth - 9 * 8, 24, "following", 1.0F );

	x = 0.5f * ( vScreen.width - GIANT_WIDTH * CG_DrawStrlen( name ) );

	CG_DrawStringExt( x, 40, name, colorWhite, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	const char	*s;
	int			w;

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.lowAmmoWarning ) {
		return;
	}

	if ( cg.scoreBoardShowing ) {
		return;
	}

	if ( cg.lowAmmoWarning == 2 ) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}

	if (hud.file) {
		CGX_DrawString(XH_AmmoMessage, s, 0);
		return;
	}

	w = CG_DrawStrlen( s ) * hud.big_char_w;
	CG_DrawBigString2(vScreen.hwidth - w / 2, 64, s, 1.0F);
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			w;
	int			sec;
	int			i;
	clientInfo_t	*ci1, *ci2;
	int			cw;
	const char	*s;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";		
		if (hud.file) {
			CGX_DrawString(XH_WarmupInfo, s, 0);
			return;
		}
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		CG_DrawBigString(vScreen.hwidth - w / 2, 40, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
				if ( !ci1 ) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if ( ci1 && ci2 ) {
			s = va( "^7%s ^7vs %s", ci1->name, ci2->name );
			if (hud.file) {
				CGX_DrawString(XH_GameType, s, 0);
			} else {
				w = CG_DrawStrlen(s);
				if (w > vScreen.width / hud.giant_char_w) {
					cw = vScreen.width / w;
				} else {
					cw = hud.giant_char_w;
				}
				CG_DrawStringExt(vScreen.hwidth - w * cw / 2, 20, s, colorWhite,
					qfalse, qtrue, cw, (int)(cw * 1.5f), -1);
			}
		}
	} else {
		if ( cgs.gametype == GT_FFA ) {
			s = "Free For All";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = "Team Deathmatch";
		} else if ( cgs.gametype == GT_CTF ) {
			s = "Capture the Flag";
		} else {
			s = "";
		}
		if (hud.file) {
			CGX_DrawString(XH_GameType, s, 0);
		} else {
			w = CG_DrawStrlen( s );
			if ( w > vScreen.width / hud.giant_char_w ) {
				cw = vScreen.width / w;
			} else {
				cw = hud.giant_char_w;
			}
			CG_DrawStringExt( vScreen.hwidth - w * cw/2, 20,s, colorWhite, 
					qfalse, qtrue, cw, (int)(cw * 1.5f), 0 );
		}
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}
	if (hud.file) {
		CGX_DrawString(XH_WarmupInfo, s, 0);
		return;
	}
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		break;
	case 1:
		cw = 24;
		break;
	case 2:
		cw = 20;
		break;
	default:
		cw = 16;
		break;
	}

	if (hud.type == HUD_COMPACT) {
		cw /= 1.33333333f;
		i = 60;
	} else {
		i = 70;
	}
	
	w = CG_DrawStrlen( s );
	CG_DrawStringExt( vScreen.hwidth - w * cw/2, i, s, colorWhite, 
			qfalse, qtrue, cw, (int)(cw * 1.5f), -1 );
}

//==================================================================================

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void ) {

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if (cg_draw2D.integer == 0 ) {
		return;
	}

	CGX_DrawHUDLayer(-1);

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		// OSP window style engine
		CG_windowDraw();
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	} else {
		// don't draw any status if dead
		if ( cg.snap->ps.stats[STAT_HEALTH] > 0 ) {
			CGX_DrawHUDLayer(XH_PreDecorate);

			CG_DrawStatusBar();
			CG_DrawAmmoWarning();
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
			CG_DrawHoldableItem();
			CG_DrawReward();

			CGX_DrawHUDLayer(XH_PostDecorate);
		}
		if ( cgs.gametype >= GT_TEAM ) {
			CG_DrawTeamInfo();
		}
	}

	CG_DrawVote();

	CG_DrawLagometer();

	CG_DrawUpperRight();

	CG_DrawLowerRight();

	CG_DrawLowerLeft();

	if ( !CG_DrawFollow() ) {
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	if ( !( cg.scoreBoardShowing = CG_DrawScoreboard() ) ) {
		CG_DrawCenterString();
	}

	// OSP window style engine
	CG_windowDraw();
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
		CG_DrawTourneyScoreboard();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	// draw 3D view
	trap_R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

	// draw status bar and other floating elements
	CG_Draw2D();
}

