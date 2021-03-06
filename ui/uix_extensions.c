// Copyright (C) 2018 NaViGaToR (322)

#include "ui_local.h"

extern void Controls_GetKeyAssignment(char *command, int *twokeys);

/* X-MOD: commonstatusbar for quick infos */
void UIX_CommonStatusBar(void *self, int min, int total, const char* info_messages[][2]) {
	const char *infomsg, *infomsg2;

	int id = ((menucommon_s*)self)->id;	
	if (id >= min && id < min + total) {
		infomsg = info_messages[id - min][0];
		infomsg2 = info_messages[id - min][1];
	} else {
		infomsg = va("Unknown id %i", id);
		infomsg2 = "";
	}

	if (!infomsg[0])
		infomsg = va("%i empty", id);

	if (!infomsg2[0]) {
		UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.94, infomsg, UI_SMALLFONT | UI_CENTER, colorWhite);
	} else {
		UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.92, infomsg, UI_SMALLFONT | UI_CENTER, colorWhite);
		UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.96, infomsg2, UI_SMALLFONT | UI_CENTER, colorWhite);
	}
}

void UIX_DrawStringInScroll( int x, int y, const char* str, int style, vec4_t color, int y_pos, int y_max, int y1, int y2 ) {
	if (y >= y1 && y <= y2) {
		vec4_t c1;
		Vector4Copy(color, c1);
		if ((y_pos < 0 && y <= y1 + SMALLCHAR_HEIGHT) ||
			(y_pos > y_max && y >= y2 - SMALLCHAR_HEIGHT))
			c1[3] = 0.66f;
		UI_DrawString(x, y, str, style, c1);
	}
}

//gets picmip and save its value
int UIX_GetPicmip() {	
	static int val = -1;
	if (val == -1) {
		char buf[4];
		trap_Cvar_VariableStringBuffer("r_picmip", buf, sizeof(buf));		
		val = atoi(buf);
		//save for cg_nomip
		if (uix_nomip.integer && val)
			trap_Cvar_Set("cgx_r_picmip", buf);
	}
	return val;
}

static void UIX_NomipStart() {	
	if (uix_nomip.integer && UIX_GetPicmip())
		trap_Cvar_Set("r_picmip", "0");
}

static void UIX_NomipEnd() {		
	if (uix_nomip.integer && UIX_GetPicmip()) {
		char buf[4];
		trap_Cvar_VariableStringBuffer("cgx_r_picmip", buf, sizeof(buf));		
		trap_Cvar_Set("r_picmip", buf);
	}
}

void UIX_PlayerInfo_SetModel( playerInfo_t *pi, const char *model ) {
	if( trap_MemoryRemaining() <= LOW_MEMORY )
		return;

	UIX_NomipStart();
	UI_PlayerInfo_SetModel(pi, model);
	UIX_NomipEnd();
}

void UIX_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
	UIX_NomipStart();
	UI_PlayerInfo_SetWeapon(pi, weaponNum);
	UIX_NomipEnd();
}

//get +scores key for cgame
void UIX_Init_Input() {
	int keys[2];
	char name[32];
	
	Controls_GetKeyAssignment("+scores", keys);

	if (keys[0] != -1)
		trap_Key_KeynumToStringBuf(keys[0], name, 32);
	else
		*name = 0;

	trap_Cvar_Set("cgx_scores_key", name);
}

void UIX_GetCDKey(char *buf, int buflen) {
	if (uis.q3version > 11)
		trap_GetCDKey(buf, buflen);
	else
		trap_Cvar_VariableStringBuffer("cl_cdkey", buf, buflen);

	if (!*buf || !strcmp(buf, "123456789"))
		strcpy(buf, "3222222222222222");
}

void UIX_SetCDKey(char *buf) {
	if (uis.q3version > 11)
		trap_SetCDKey(buf);
	else
		trap_Cmd_ExecuteText(EXEC_APPEND, va("seta cl_cdkey %s\n", buf));
}

//return api version based on game version
//seems the only difference between 1.11 and 1.16n ui is cd key traps
//mods menu can work in 1.11, but it's hidden on purpose
int UIX_GetApiVersion() {
	char buf[32];
	
	trap_Cvar_VariableStringBuffer("version", buf, sizeof buf);

	uis.q3version = atoi(&buf[5]);

	//trap_Print(va("^3version: %s uis.q3version %i\n", buf, uis.q3version));

	if (uis.q3version > 11)
		return UI_API_VERSION;

	return UI_API_VERSION - 1;
}

void UIX_Init() {
	UIX_Init_Input();
}
