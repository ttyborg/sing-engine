/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "mathlib.h"
#include "extdll.h"
#include "const.h"
#include "basemenu.h"
#include "utils.h"
#include "keydefs.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "menu_btnsbmp_table.h"

#define ART_BANNER		"gfx/shell/head_customize"

#define ID_BACKGROUND	0
#define ID_BANNER		1
#define ID_DONE		2
#define ID_ADVOPTIONS	3
#define ID_VIEW		4
#define ID_NAME		5
#define ID_MODEL		6
#define ID_TOPCOLOR		7
#define ID_BOTTOMCOLOR	8
#define ID_HIMODELS		9
#define ID_SPECTATOR	10

#define MAX_PLAYERMODELS	100

typedef struct
{
	char		models[MAX_PLAYERMODELS][CS_SIZE];
	int		num_models;
	char		currentModel[CS_SIZE];

	ref_params_t	refdef;
	cl_entity_t	*ent;
	
	menuFramework_s	menu;

	menuBitmap_s	background;
	menuBitmap_s	banner;

	menuPicButton_s	done;
	menuPicButton_s	AdvOptions;
	menuBitmap_s	view;

	menuCheckBox_s	hiModels;
	menuSlider_s	topColor;
	menuSlider_s	bottomColor;
	menuCheckBox_s	spectator;

	menuField_s	name;
	menuSpinControl_s	model;
} uiPlayerSetup_t;

static uiPlayerSetup_t	uiPlayerSetup;

/*
=================
UI_PlayerSetup_CalcFov

assume refdef is valid
=================
*/
static void UI_PlayerSetup_CalcFov( ref_params_t *fd )
{
	float x = fd->viewport[2] / tan( DEG2RAD( fd->fov_x ) * 0.5f );
	float half_fov_y = atan( fd->viewport[3] / x );
	fd->fov_y = RAD2DEG( half_fov_y ) * 2;
}

/*
=================
UI_PlayerSetup_FindModels
=================
*/
static void UI_PlayerSetup_FindModels( void )
{
	char	name[256], path[256];
	char	**filenames;
	int	i, numFiles;
	
	uiPlayerSetup.num_models = 0;

	// Get file list
	filenames = FS_SEARCH(  "models/player/*", &numFiles, TRUE );
	if( !numFiles ) filenames = FS_SEARCH(  "models/player/*", &numFiles, FALSE ); 
#if 1
	// add default singleplayer model
	strcpy( uiPlayerSetup.models[uiPlayerSetup.num_models], "player" );
	uiPlayerSetup.num_models++;
#endif
	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		COM_FileBase( filenames[i], name );
		sprintf( path, "models/player/%s/%s.mdl", name, name );
		if( !FILE_EXISTS( path )) continue;

		strcpy( uiPlayerSetup.models[uiPlayerSetup.num_models], name );
		uiPlayerSetup.num_models++;
	}
}

/*
=================
UI_PlayerSetup_GetConfig
=================
*/
static void UI_PlayerSetup_GetConfig( void )
{
	int	i;

	strncpy( uiPlayerSetup.name.buffer, CVAR_GET_STRING( "name" ), sizeof( uiPlayerSetup.name.buffer ));

	// find models
	UI_PlayerSetup_FindModels();

	// select current model
	for( i = 0; i < uiPlayerSetup.num_models; i++ )
	{
		if( !stricmp( uiPlayerSetup.models[i], CVAR_GET_STRING( "model" )))
		{
			uiPlayerSetup.model.curValue = (float)i;
			break;
		}
	}

	strcpy( uiPlayerSetup.currentModel, uiPlayerSetup.models[(int)uiPlayerSetup.model.curValue] );
	uiPlayerSetup.model.maxValue = (float)(uiPlayerSetup.num_models - 1);

	uiPlayerSetup.topColor.curValue = CVAR_GET_FLOAT( "topcolor" ) / 255;
	uiPlayerSetup.bottomColor.curValue = CVAR_GET_FLOAT( "bottomcolor" ) / 255;

	if( CVAR_GET_FLOAT( "cl_himodels" ))
		uiPlayerSetup.hiModels.enabled = 1;

	if( CVAR_GET_FLOAT( "spectator" ))
		uiPlayerSetup.spectator.enabled = 1;
}

/*
=================
UI_PlayerSetup_SetConfig
=================
*/
static void UI_PlayerSetup_SetConfig( void )
{
	CVAR_SET_STRING( "name", uiPlayerSetup.name.buffer );
	CVAR_SET_STRING( "model", uiPlayerSetup.currentModel );
	CVAR_SET_FLOAT( "topcolor", uiPlayerSetup.topColor.curValue * 255 );
	CVAR_SET_FLOAT( "bottomcolor", uiPlayerSetup.bottomColor.curValue * 255 );
	CVAR_SET_FLOAT( "cl_himodels", uiPlayerSetup.hiModels.enabled );
	CVAR_SET_FLOAT( "spectator", uiPlayerSetup.spectator.enabled );
}

/*
=================
UI_PlayerSetup_UpdateConfig
=================
*/
static void UI_PlayerSetup_UpdateConfig( void )
{
	char	path[256], name[256];

	// see if the model has changed
	if( stricmp( uiPlayerSetup.currentModel, uiPlayerSetup.models[(int)uiPlayerSetup.model.curValue] ))
	{
		strcpy( uiPlayerSetup.currentModel, uiPlayerSetup.models[(int)uiPlayerSetup.model.curValue] );
	}

	uiPlayerSetup.model.generic.name = uiPlayerSetup.models[(int)uiPlayerSetup.model.curValue];
	strcpy( name, uiPlayerSetup.models[(int)uiPlayerSetup.model.curValue] );

	if( !stricmp( name, "player" ))
		strcpy( path, "models/player.mdl" );
	else sprintf( path, "models/player/%s/%s.mdl", name, name );

	CVAR_SET_STRING( "model", uiPlayerSetup.currentModel );
	CVAR_SET_FLOAT( "cl_himodels", uiPlayerSetup.hiModels.enabled );
	CVAR_SET_FLOAT( "spectator", uiPlayerSetup.spectator.enabled );

	// IMPORTANT: always set default model becuase we need to have something valid here
	// if you wish draw your playermodel as normal studiomodel please change "models/player.mdl" to path
	if( uiPlayerSetup.ent )
		ENGINE_SET_MODEL( uiPlayerSetup.ent, "models/player.mdl" );
}

/*
=================
UI_PlayerSetup_Callback
=================
*/
static void UI_PlayerSetup_Callback( void *self, int event )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	switch( item->id )
	{
	case ID_HIMODELS:
	case ID_SPECTATOR:
		if( event == QM_PRESSED )
			((menuCheckBox_s *)self)->focusPic = UI_CHECKBOX_PRESSED;
		else ((menuCheckBox_s *)self)->focusPic = UI_CHECKBOX_FOCUS;
		break;
	}

	if( event == QM_CHANGED )
	{
		UI_PlayerSetup_UpdateConfig();
		return;
	}

	if( event != QM_ACTIVATED )
		return;

	switch( item->id )
	{
	case ID_DONE:
		UI_PlayerSetup_SetConfig();
		UI_PopMenu();
		break;
	case ID_ADVOPTIONS:
		UI_PlayerSetup_SetConfig();
		UI_GameOptions_Menu();
		break;
	}
}

/*
=================
UI_PlayerSetup_Ownerdraw
=================
*/
static void UI_PlayerSetup_Ownerdraw( void *self )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	// draw the background
	UI_FillRect( item->x, item->y, item->width, item->height, uiPromptBgColor );

	// draw the rectangle
	UI_DrawRectangle( item->x, item->y, item->width, item->height, uiInputFgColor );

	R_ClearScene ();

	// update renderer timings
	uiPlayerSetup.refdef.time = gpGlobals->time;
	uiPlayerSetup.refdef.frametime = gpGlobals->frametime;
	uiPlayerSetup.ent->curstate.body = 0; // clearing body each frame

	// draw the player model
	R_AddEntity( ET_NORMAL, uiPlayerSetup.ent );
	R_RenderFrame( &uiPlayerSetup.refdef );
}

/*
=================
UI_PlayerSetup_Init
=================
*/
static void UI_PlayerSetup_Init( void )
{
	bool game_hlRally = FALSE;

	memset( &uiPlayerSetup, 0, sizeof( uiPlayerSetup_t ));

	// disable playermodel preview for HLRally to prevent crash
	if( !stricmp( gMenu.m_gameinfo.gamefolder, "hlrally" ))
		game_hlRally = TRUE;

	uiPlayerSetup.menu.vidInitFunc = UI_PlayerSetup_Init;

	uiPlayerSetup.background.generic.id = ID_BACKGROUND;
	uiPlayerSetup.background.generic.type = QMTYPE_BITMAP;
	uiPlayerSetup.background.generic.flags = QMF_INACTIVE;
	uiPlayerSetup.background.generic.x = 0;
	uiPlayerSetup.background.generic.y = 0;
	uiPlayerSetup.background.generic.width = 1024;
	uiPlayerSetup.background.generic.height = 768;
	uiPlayerSetup.background.pic = ART_BACKGROUND;

	uiPlayerSetup.banner.generic.id = ID_BANNER;
	uiPlayerSetup.banner.generic.type = QMTYPE_BITMAP;
	uiPlayerSetup.banner.generic.flags = QMF_INACTIVE|QMF_DRAW_ADDITIVE;
	uiPlayerSetup.banner.generic.x = UI_BANNER_POSX;
	uiPlayerSetup.banner.generic.y = UI_BANNER_POSY;
	uiPlayerSetup.banner.generic.width = UI_BANNER_WIDTH;
	uiPlayerSetup.banner.generic.height = UI_BANNER_HEIGHT;
	uiPlayerSetup.banner.pic = ART_BANNER;

	uiPlayerSetup.done.generic.id = ID_DONE;
	uiPlayerSetup.done.generic.type = QMTYPE_BM_BUTTON;
	uiPlayerSetup.done.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.done.generic.x = 72;
	uiPlayerSetup.done.generic.y = 230;
	uiPlayerSetup.done.generic.name = "Done";
	uiPlayerSetup.done.generic.statusText = "Go back to the Multiplayer Menu";
	uiPlayerSetup.done.generic.callback = UI_PlayerSetup_Callback;

	UI_UtilSetupPicButton( &uiPlayerSetup.done, PC_DONE );

	uiPlayerSetup.AdvOptions.generic.id = ID_ADVOPTIONS;
	uiPlayerSetup.AdvOptions.generic.type = QMTYPE_BM_BUTTON;
	uiPlayerSetup.AdvOptions.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.AdvOptions.generic.x = 72;
	uiPlayerSetup.AdvOptions.generic.y = 280;
	uiPlayerSetup.AdvOptions.generic.name = "Adv. Options";
	uiPlayerSetup.AdvOptions.generic.statusText = "Configure handness, fov and other advanced options";
	uiPlayerSetup.AdvOptions.generic.callback = UI_PlayerSetup_Callback;

	UI_UtilSetupPicButton( &uiPlayerSetup.AdvOptions, PC_ADV_OPT );

	uiPlayerSetup.view.generic.id = ID_VIEW;
	uiPlayerSetup.view.generic.type = QMTYPE_BITMAP;
	uiPlayerSetup.view.generic.flags = QMF_INACTIVE;
	uiPlayerSetup.view.generic.x = 660;
	uiPlayerSetup.view.generic.y = 260;
	uiPlayerSetup.view.generic.width = 260;
	uiPlayerSetup.view.generic.height = 320;
	uiPlayerSetup.view.generic.ownerdraw = UI_PlayerSetup_Ownerdraw;

	uiPlayerSetup.name.generic.id = ID_NAME;
	uiPlayerSetup.name.generic.type = QMTYPE_FIELD;
	uiPlayerSetup.name.generic.flags = QMF_CENTER_JUSTIFY|QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.name.generic.x = 320;
	uiPlayerSetup.name.generic.y = 260;
	uiPlayerSetup.name.generic.width = 256;
	uiPlayerSetup.name.generic.height = 36;
	uiPlayerSetup.name.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.name.generic.statusText = "Enter your multiplayer display name";
	uiPlayerSetup.name.maxLength = 32;

	uiPlayerSetup.model.generic.id = ID_MODEL;
	uiPlayerSetup.model.generic.type = QMTYPE_SPINCONTROL;
	uiPlayerSetup.model.generic.flags = QMF_CENTER_JUSTIFY|QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.model.generic.x = game_hlRally ? 320 : 702;
	uiPlayerSetup.model.generic.y = game_hlRally ? 320 : 590;
	uiPlayerSetup.model.generic.width = game_hlRally ? 256 : 176;
	uiPlayerSetup.model.generic.height = game_hlRally ? 36 : 32;
	uiPlayerSetup.model.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.model.generic.statusText = "Select a model for representation in multiplayer";
	uiPlayerSetup.model.minValue = 0;
	uiPlayerSetup.model.maxValue = 1;
	uiPlayerSetup.model.range  = 1;

	uiPlayerSetup.topColor.generic.id = ID_TOPCOLOR;
	uiPlayerSetup.topColor.generic.type = QMTYPE_SLIDER;
	uiPlayerSetup.topColor.generic.flags = QMF_PULSEIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.topColor.generic.name = "Top color";
	uiPlayerSetup.topColor.generic.x = 350;
	uiPlayerSetup.topColor.generic.y = 550;
	uiPlayerSetup.topColor.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.topColor.generic.statusText = "Set a player model top color";
	uiPlayerSetup.topColor.minValue = 0.0;
	uiPlayerSetup.topColor.maxValue = 1.0;
	uiPlayerSetup.topColor.range = 0.05f;

	uiPlayerSetup.bottomColor.generic.id = ID_BOTTOMCOLOR;
	uiPlayerSetup.bottomColor.generic.type = QMTYPE_SLIDER;
	uiPlayerSetup.bottomColor.generic.flags = QMF_PULSEIFFOCUS|QMF_DROPSHADOW;
	uiPlayerSetup.bottomColor.generic.name = "Bottom color";
	uiPlayerSetup.bottomColor.generic.x = 350;
	uiPlayerSetup.bottomColor.generic.y = 620;
	uiPlayerSetup.bottomColor.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.bottomColor.generic.statusText = "Set a player model bottom color";
	uiPlayerSetup.bottomColor.minValue = 0.0;
	uiPlayerSetup.bottomColor.maxValue = 1.0;
	uiPlayerSetup.bottomColor.range = 0.05f;

	uiPlayerSetup.hiModels.generic.id = ID_HIMODELS;
	uiPlayerSetup.hiModels.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.hiModels.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.hiModels.generic.name = "High quality models";
	uiPlayerSetup.hiModels.generic.x = 72;
	uiPlayerSetup.hiModels.generic.y = 380;
	uiPlayerSetup.hiModels.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.hiModels.generic.statusText = "show hi-res models in multiplayer";

	uiPlayerSetup.spectator.generic.id = ID_SPECTATOR;
	uiPlayerSetup.spectator.generic.type = QMTYPE_CHECKBOX;
	uiPlayerSetup.spectator.generic.flags = QMF_HIGHLIGHTIFFOCUS|QMF_ACT_ONRELEASE|QMF_MOUSEONLY|QMF_DROPSHADOW;
	uiPlayerSetup.spectator.generic.name = "Play as a spectator";
	uiPlayerSetup.spectator.generic.x = 72;
	uiPlayerSetup.spectator.generic.y = 430;
	uiPlayerSetup.spectator.generic.callback = UI_PlayerSetup_Callback;
	uiPlayerSetup.spectator.generic.statusText = "enable spectator mode in multiplayer";

	UI_PlayerSetup_GetConfig();

	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.background );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.banner );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.done );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.AdvOptions );
	// disable playermodel preview for HLRally to prevent crash
	if( game_hlRally == FALSE )
		UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.view );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.name );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.model );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.topColor );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.bottomColor );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.hiModels );
	UI_AddItem( &uiPlayerSetup.menu, (void *)&uiPlayerSetup.spectator );

	// setup render and actor
	uiPlayerSetup.refdef.fov_x = 40;

	// NOTE: must be called after UI_AddItem whan we sure what UI_ScaleCoords is done
	uiPlayerSetup.refdef.viewport[0] = uiPlayerSetup.view.generic.x + (uiPlayerSetup.view.generic.width / 12);
	uiPlayerSetup.refdef.viewport[1] = uiPlayerSetup.view.generic.y + (uiPlayerSetup.view.generic.height / 12);
	uiPlayerSetup.refdef.viewport[2] = uiPlayerSetup.view.generic.width-(uiPlayerSetup.view.generic.width / 6);
	uiPlayerSetup.refdef.viewport[3] = uiPlayerSetup.view.generic.height-(uiPlayerSetup.view.generic.height / 6);

	UI_PlayerSetup_CalcFov( &uiPlayerSetup.refdef );
	uiPlayerSetup.ent = GET_MENU_EDICT ();

	if( !uiPlayerSetup.ent )
		return;

	// adjust entity params
	uiPlayerSetup.ent->curstate.number = 1;	// IMPORTANT: always set playerindex to 1
	uiPlayerSetup.ent->curstate.animtime = gpGlobals->time;	// start animation
	uiPlayerSetup.ent->curstate.sequence = 1;
	uiPlayerSetup.ent->curstate.scale = 1.0f;
	uiPlayerSetup.ent->curstate.frame = 0.0f;
	uiPlayerSetup.ent->curstate.framerate = 1.0f;
	uiPlayerSetup.ent->curstate.effects |= EF_FULLBRIGHT;
	uiPlayerSetup.ent->curstate.controller[0] = 127;
	uiPlayerSetup.ent->curstate.controller[1] = 127;
	uiPlayerSetup.ent->curstate.controller[2] = 127;
	uiPlayerSetup.ent->curstate.controller[3] = 127;
	uiPlayerSetup.ent->latched.prevcontroller[0] = 127;
	uiPlayerSetup.ent->latched.prevcontroller[1] = 127;
	uiPlayerSetup.ent->latched.prevcontroller[2] = 127;
	uiPlayerSetup.ent->latched.prevcontroller[3] = 127;
	uiPlayerSetup.ent->origin[0] = uiPlayerSetup.ent->curstate.origin[0] = 92;
	uiPlayerSetup.ent->origin[2] = uiPlayerSetup.ent->curstate.origin[2] = 2;
	uiPlayerSetup.ent->angles[1] = uiPlayerSetup.ent->curstate.angles[1] = 180;
	uiPlayerSetup.ent->player = true; // yes, draw me as playermodel
}

/*
=================
UI_PlayerSetup_Precache
=================
*/
void UI_PlayerSetup_Precache( void )
{
	PIC_Load( ART_BACKGROUND );
	PIC_Load( ART_BANNER );
}

/*
=================
UI_PlayerSetup_Menu
=================
*/
void UI_PlayerSetup_Menu( void )
{
	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		return;

	UI_PlayerSetup_Precache();
	UI_PlayerSetup_Init();

	UI_PlayerSetup_UpdateConfig();
	UI_PushMenu( &uiPlayerSetup.menu );
}