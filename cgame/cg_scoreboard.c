// Copyright (C) 1999-2000 Id Software, Inc.
// Copyright (C) 2018 NaViGaToR (322)
//
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"


#define	SCOREBOARD_X		(16)

#define SB_HEADER			86
#define SB_TOP				(SB_HEADER+32)

// Where the status bar starts, so we don't overwrite it
#define SB_STATUSBAR		420

#define SB_NORMAL_HEIGHT	40
#define SB_INTER_HEIGHT		16 // interleaved height

#define SB_MAXCLIENTS_NORMAL  ((SB_STATUSBAR - SB_TOP) / SB_NORMAL_HEIGHT)
#define SB_MAXCLIENTS_INTER   ((SB_STATUSBAR - SB_TOP) / SB_INTER_HEIGHT - 1)

// Used when interleaved



#define SB_LEFT_BOTICON_X	(SCOREBOARD_X+0)
#define SB_LEFT_HEAD_X		(SCOREBOARD_X+32)
#define SB_RIGHT_BOTICON_X	(SCOREBOARD_X+64)
#define SB_RIGHT_HEAD_X		(SCOREBOARD_X+96)
// Normal
#define SB_BOTICON_X		(SCOREBOARD_X+32)
#define SB_HEAD_X			(SCOREBOARD_X+64)

#define SB_SCORELINE_X		128

#define SB_SCORE_X			(SB_SCORELINE_X + BIGCHAR_WIDTH) // width 6
#define SB_PING_X			(SB_SCORELINE_X + 6 * BIGCHAR_WIDTH + 8) // width 5
#define SB_TIME_X			(SB_SCORELINE_X + 11 * BIGCHAR_WIDTH + 8) // width 5
#define SB_NAME_X			(SB_SCORELINE_X + 16 * BIGCHAR_WIDTH) // width 15

// The new and improved score board
//
// In cases where the number of clients is high, the score board heads are interleaved
// here's the layout

//
//	0   32   80  112  144   240  320  400   <-- pixel position
//  bot head bot head score ping time name
//  
//  wins/losses are drawn on bot icon now

static qboolean localClient; // true if local client has been displayed


/*
=================
CG_DrawScoreboard
=================
*/
static void CG_DrawClientScore( int y, score_t *score, float *color, float fade, qboolean largeFormat ) {
	char	string[1024];
	vec3_t	headAngles;
	clientInfo_t	*ci;
	int iconx, headx, iconxoffs, w;
	int		pingCol;

	if ( score->client < 0 || score->client >= cgs.maxclients ) {
		Com_Printf( "Bad score->client: %i\n", score->client );
		return;
	}
	
	ci = &cgs.clientinfo[score->client];

	iconx = SB_BOTICON_X + vScreen.offsetx;
	headx = SB_HEAD_X + vScreen.offsetx;
	//iconxoffs = cgx_scoreboard.integer < 2 ? -16 : 0;
	iconxoffs = -20;

	// draw the handicap or bot skill marker (unless player has flag)
	if ( ci->powerups & ( 1 << PW_REDFLAG ) ) {
		if( largeFormat ) {
			CG_DrawFlagModel( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, TEAM_RED );
		}
		else {
			CG_DrawFlagModel( iconx - iconxoffs * 1.75f, y, 16, 16, TEAM_RED );
		}
	} else if ( ci->powerups & ( 1 << PW_BLUEFLAG ) ) {
		if( largeFormat ) {
			CG_DrawFlagModel( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, TEAM_BLUE );
		}
		else {
			CG_DrawFlagModel( iconx - iconxoffs * 1.75f, y, 16, 16, TEAM_BLUE );
		}
	} else {
		if ( ci->botSkill > 0 && ci->botSkill <= 5 ) {
			if ( cg_drawIcons.integer ) {
				if( largeFormat ) {
					CG_DrawPic( iconx, y - ( 32 - BIGCHAR_HEIGHT ) / 2, 32, 32, cgs.media.botSkillShaders[ ci->botSkill - 1 ] );
				}
				else {
					CG_DrawPic( iconx - iconxoffs * 1.75f, y, 16, 16, cgs.media.botSkillShaders[ ci->botSkill - 1 ] );
				}
			}
		} else if ( ci->handicap < 100 ) {
			Com_sprintf( string, sizeof( string ), "%i", ci->handicap );
			//if ( cgs.gametype == GT_TOURNAMENT )			
			//	CG_DrawSmallStringColor( iconx - iconxoffs * 1.75f, y, string, color );
			//else
			//	CG_DrawSmallStringColor( iconx - iconxoffs * 1.75f, y, string, color );
			if( largeFormat )
				CG_DrawSmallStringColor( iconx + 8, y + 16, string, color );
			else
				CG_DrawSmallStringColor( iconx - iconxoffs * 1.75f, y, string, color );		
		}

		// draw the wins / losses
		if ( cgs.gametype == GT_TOURNAMENT ) {
			Com_sprintf( string, sizeof( string ), "%i/%i", ci->wins, ci->losses );
			w = strlen( string ) * SMALLCHAR_WIDTH;
			//if( ci->handicap < 100 && !ci->botSkill ) {
			//	CG_DrawSmallStringColor( iconx - iconxoffs * 0.75f - 34, y, string, color );
			//}
			//else {
			//	CG_DrawSmallStringColor( iconx - iconxoffs * 0.75f - 34, y, string, color );
			//}

			CG_DrawSmallStringColor( iconx - iconxoffs * 0.75f - w - 10, y, string, color );
		}
	}

	// draw the face
	VectorClear( headAngles );
	headAngles[YAW] = 180;
	if( largeFormat ) {
		CG_DrawHead( headx, y - ( ICON_SIZE - BIGCHAR_HEIGHT ) / 2, ICON_SIZE, ICON_SIZE, 
			score->client, headAngles );
	}
	else {
		CG_DrawHead( headx - iconxoffs, y, 16, 16, score->client, headAngles );
	}

	// X-MOD: draw player ID

	if (cgx_scoreboard.integer != 2) {		
		float color[4];
		color[0] = color[1] = color[2] = 1.0f;
		Com_sprintf(string, sizeof(string), "%i", score->client);
		w = (strlen( string ) - 1) * BIGCHAR_WIDTH / 2;
		if (largeFormat) {
			color[3] = 0.5f;
			CG_DrawStringExt( SB_SCORELINE_X + vScreen.offsetx - 64 - 8 - w, y, string,
				color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
			if (score->isReferee) {
				color[3] = 1.0f;
				CG_DrawStringExt( iconx + 8, y - 16, "^2R", color, qfalse, qtrue, 
					BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
			}
		} else {
			color[3] = 0.33f;
			CG_DrawStringExt( SB_SCORELINE_X + vScreen.offsetx - 64 - 4 - w, y, string, 
				color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_WIDTH, 0 );
			if (score->isReferee) {
				color[3] = 1.0f;
				CG_DrawStringExt( headx + 40, y, "^2R", color, qfalse, qtrue, 
					BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
			}
		}
	}

	// X-MOD: color ping
	if (cgx_scoreboard.integer == 2)
		pingCol = ColorIndex(COLOR_WHITE);
	else if (score->ping <= 50)
		pingCol = ColorIndex(COLOR_WHITE);
	else if (score->ping <= 100)
		pingCol = ColorIndex(COLOR_GREEN);
	else if (score->ping <= 250)
		pingCol = ColorIndex(COLOR_YELLOW);
	else if (score->ping <= 400)
		pingCol = ColorIndex(COLOR_MAGENTA);
	else
		pingCol = ColorIndex(COLOR_RED);

	// draw the score line
	if ( score->ping == -1 ) {
		Com_sprintf(string, sizeof(string),
			" connecting    %s", ci->name);
	} else if ( ci->team == TEAM_SPECTATOR ) {
		Com_sprintf(string, sizeof(string),
			" SPECT ^%d%3i ^7%4i %s", pingCol, score->ping, score->time, ci->name);
	} else {
		Com_sprintf(string, sizeof(string),
			"%5i ^%i%4i ^7%4i %s", score->score, pingCol, score->ping, score->time, ci->name);
	}

	// highlight your position
	if ( score->client == cg.snap->ps.clientNum ) {
		float	hcolor[4];
		int		rank;

		localClient = qtrue;

		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR 
			|| cgs.gametype >= GT_TEAM ) {
			rank = -1;
		} else {
			rank = cg.snap->ps.persistant[PERS_RANK] & ~RANK_TIED_FLAG;
		}
		if ( rank == 0 ) {
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 0.7;
		} else if ( rank == 1 ) {
			hcolor[0] = 0.7;
			hcolor[1] = 0;
			hcolor[2] = 0;
		} else if ( rank == 2 ) {
			hcolor[0] = 0.7;
			hcolor[1] = 0.7;
			hcolor[2] = 0;
		} else {
			hcolor[0] = 0.7;
			hcolor[1] = 0.7;
			hcolor[2] = 0.7;
		}

		hcolor[3] = fade * 0.7;
		CG_FillRect( SB_SCORELINE_X + BIGCHAR_WIDTH + vScreen.offsetx, y, 
			640 - SB_SCORELINE_X - BIGCHAR_WIDTH, BIGCHAR_HEIGHT+1, hcolor );
	}

	//CG_DrawBigString( SB_SCORELINE_X + vScreen.offsetx, y, string, fade );
	//x-mod: cut names on wide screens
	{
		float	fcolor[4];

		fcolor[0] = fcolor[1] = fcolor[2] = 1.0;
		fcolor[3] = fade;
		CG_DrawStringExt( SB_SCORELINE_X + vScreen.offsetx, y, string, fcolor, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 32 );
	}

	// add the "ready" marker for intermission exiting
	if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << score->client ) ) {
#if CGX_FREEZE//freeze
		if ( !Q_Isfreeze( score->client ) )
#endif//freeze
		CG_DrawBigStringColor( iconx, y, "READY", color );
	}
}

/*
=================
CG_TeamScoreboard
=================
*/
static int CG_TeamScoreboard( int y, team_t team, float fade, int maxClients, int lineHeight ) {
	int		i;
	score_t	*score;
	float	color[4];
	int		count;
	clientInfo_t	*ci;

	color[0] = color[1] = color[2] = 1.0;
	color[3] = fade;

	count = 0;

	for ( i = 0 ; i < cg.numScores && count < maxClients ; i++ ) {
		score = &cg.scores[i];
		ci = &cgs.clientinfo[ score->client ];

		if ( team != ci->team ) {
			continue;
		}

		CG_DrawClientScore( y + lineHeight * count, score, color, fade, lineHeight == SB_NORMAL_HEIGHT );

		count++;
	}

	return count;
}

/*
=================
CG_DrawNormalScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawNormalScoreboard( void ) {
	int		x, y, w, i, n1, n2;
	float	fade;
	float	*fadeColor;
	char	*s;
	int maxClients;
	int lineHeight;
	int topBorderSize, bottomBorderSize;
	int sb_header_ofs;

	// don't draw amuthing if the menu or console is up
	if ( cg_paused.integer ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	if ( cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		cg.deferredPlayerLoading = 0;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores && cg.predictedPlayerState.pm_type != PM_INTERMISSION ) {
		return qfalse;
	}

	if ( cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD ||
		 cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		
		if ( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			return qfalse;
		}
		fade = *fadeColor;
	}

	sb_header_ofs = cg.numScores > SB_MAXCLIENTS_INTER && cgx_scoreboard.integer != 2 ? 36 : 0;

	// fragged by ... line
	if ( cg.killerName[0] ) {
		s = va("Fragged by %s", cg.killerName );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( vScreen.width - w ) / 2;
		y = SB_HEADER - 46 - sb_header_ofs;
		CG_DrawBigString( x, y, s, fade );
	}

	// current rank
	if ( cgs.gametype < GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",
				CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
				cg.snap->ps.persistant[PERS_SCORE] );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
			x = ( vScreen.width - w ) / 2;
			y = SB_HEADER - 26 - sb_header_ofs;
			CG_DrawBigString( x, y, s, fade );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads %i to %i",cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads %i to %i",cg.teamScores[1], cg.teamScores[0] );
		}

		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		x = ( vScreen.width - w ) / 2;
		y = SB_HEADER - 26 - sb_header_ofs;
		CG_DrawBigString( x, y, s, fade );
	}

	// scoreboard
	y = SB_HEADER - sb_header_ofs;

	CG_DrawPic( SB_SCORE_X + vScreen.offsetx, y, 64, 32, cgs.media.scoreboardScore );
	CG_DrawPic( SB_PING_X  + vScreen.offsetx, y, 64, 32, cgs.media.scoreboardPing );
	CG_DrawPic( SB_TIME_X  + vScreen.offsetx, y, 64, 32, cgs.media.scoreboardTime );
	CG_DrawPic( SB_NAME_X  + vScreen.offsetx, y, 64, 32, cgs.media.scoreboardName );


	y = SB_TOP - sb_header_ofs;

	// If there are more than SB_MAXCLIENTS_NORMAL, use the interleaved scores
	if (cg.numScores > SB_MAXCLIENTS_NORMAL || cgx_scoreboard.integer == 1 || cgx_scoreboard.integer == 3) {
		maxClients = sb_header_ofs ? 
			((SB_STATUSBAR - (SB_TOP - sb_header_ofs)) / SB_INTER_HEIGHT - 1) : 
			SB_MAXCLIENTS_INTER;
		lineHeight = SB_INTER_HEIGHT;
		topBorderSize = 8;
		bottomBorderSize = 16;
	} else {
		maxClients = SB_MAXCLIENTS_NORMAL;
		lineHeight = SB_NORMAL_HEIGHT;
		topBorderSize = 16;
		bottomBorderSize = 16;
	}

	//CG_Printf("%i %i\n", maxClients, cg.numScores);

	localClient = qfalse;

	if ( cgs.gametype >= GT_TEAM ) {
		//
		// teamplay scoreboard
		//
		y += lineHeight/2;
		x = vScreen.offsetx;

		if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			n1 = CG_TeamScoreboard( y, TEAM_RED, fade, maxClients, lineHeight );
			CG_DrawTeamBackground( x, y - topBorderSize, SCREEN_WIDTH, n1 * lineHeight + bottomBorderSize, 0.33, TEAM_RED );
			y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
			maxClients -= n1;
			n2 = CG_TeamScoreboard( y, TEAM_BLUE, fade, maxClients, lineHeight );
			CG_DrawTeamBackground( x, y - topBorderSize, SCREEN_WIDTH, n2 * lineHeight + bottomBorderSize, 0.33, TEAM_BLUE );
			y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
			maxClients -= n2;
		} else {
			n1 = CG_TeamScoreboard( y, TEAM_BLUE, fade, maxClients, lineHeight );
			CG_DrawTeamBackground( x, y - topBorderSize, SCREEN_WIDTH, n1 * lineHeight + bottomBorderSize, 0.33, TEAM_BLUE );
			y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
			maxClients -= n1;
			n2 = CG_TeamScoreboard( y, TEAM_RED, fade, maxClients, lineHeight );
			CG_DrawTeamBackground( x, y - topBorderSize, SCREEN_WIDTH, n2 * lineHeight + bottomBorderSize, 0.33, TEAM_RED );
			y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
			maxClients -= n2;
		}
		n1 = CG_TeamScoreboard( y, TEAM_SPECTATOR, fade, maxClients, lineHeight );
		y += (n1 * lineHeight) + BIGCHAR_HEIGHT;

	} else {
		//
		// free for all scoreboard
		//
		n1 = CG_TeamScoreboard( y, TEAM_FREE, fade, maxClients, lineHeight );
		y += (n1 * lineHeight) + BIGCHAR_HEIGHT;
		n2 = CG_TeamScoreboard( y, TEAM_SPECTATOR, fade, maxClients - n1, lineHeight );
		y += (n2 * lineHeight) + BIGCHAR_HEIGHT;
	}

	if (!localClient) {
		// draw local client at the bottom
		for ( i = 0 ; i < cg.numScores ; i++ ) {
			if ( cg.scores[i].client == cg.snap->ps.clientNum ) {
				CG_DrawClientScore( y, &cg.scores[i], fadeColor, fade, lineHeight == SB_NORMAL_HEIGHT );
				break;
			}
		}
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 ) {				
		CG_LoadDeferredPlayers();
	}

	return qtrue;
}

/*
=================
CG_DrawScoreboard

Draw specified scoreboard
=================
*/
qboolean CG_DrawScoreboard( void ) {
	if (cgx_scoreboard.integer == 3 && cgs.gametype != GT_FFA && cgs.gametype != GT_SINGLE_PLAYER) {
		return CG_DrawOSPScoreboard();
	} else {
		return CG_DrawNormalScoreboard();
	}
}

//================================================================================

/*
================
CG_CenterGiantLine
================
*/
static void CG_CenterGiantLine( float y, const char *string ) {
	float		x;
	vec4_t		color;

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	x = 0.5 * ( vScreen.width - GIANT_WIDTH * CG_DrawStrlen( string ) );

	CG_DrawStringExt( x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
}

/*
=================
CG_DrawTourneyScoreboard

Draw the oversize scoreboard for tournements
=================
*/
void CG_DrawTourneyScoreboard( void ) {
	const char		*s;
	vec4_t			color;
	int				min, tens, ones;
	clientInfo_t	*ci;
	int				y;
	int				i;

	// request more scores regularly
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );
	}

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	// draw the dialog background
	color[0] = color[1] = color[2] = 0;
	color[3] = 1;
	CG_FillRect( 0 +vScreen.offsetx, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color );

	// print the mesage of the day
	s = CG_ConfigString( CS_MOTD );
	if ( !s[0] ) {
		s = "Scoreboard";
	}

	// print optional title
	CG_CenterGiantLine( 8, s );

	// print server time
	ones = cg.time / 1000;
	min = ones / 60;
	ones %= 60;
	tens = ones / 10;
	ones %= 10;
	s = va("%i:%i%i", min, tens, ones );

	CG_CenterGiantLine( 64, s );


	// print the two scores

	y = 160;
	if ( cgs.gametype >= GT_TEAM ) {
		//
		// teamplay scoreboard
		//
		CG_DrawStringExt( 8, y, "Red Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		s = va("%i", cg.teamScores[0] );
		CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		
		y += 64;

		CG_DrawStringExt( 8, y, "Blue Team", color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
		s = va("%i", cg.teamScores[1] );
		CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
	} else {
		//
		// free for all scoreboard
		//
		for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
			ci = &cgs.clientinfo[i];
			if ( !ci->infoValid ) {
				continue;
			}
			if ( ci->team != TEAM_FREE ) {
				continue;
			}

			CG_DrawStringExt( 8, y, ci->name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
			s = va("%i", ci->score );
			CG_DrawStringExt( 632 - GIANT_WIDTH * strlen(s), y, s, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
			y += 64;
		}
	}


}

