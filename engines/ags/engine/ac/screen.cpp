/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/ac/common.h"
#include "ags/engine/ac/draw.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/screen.h"
#include "ags/engine/ac/dynobj/scriptviewport.h"
#include "ags/engine/ac/dynobj/scriptuserobject.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/plugins/agsplugin.h"
#include "ags/plugins/plugin_engine.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;


extern IGraphicsDriver *gfxDriver;
extern AGSPlatformDriver *platform;

void my_fade_in(PALETTE p, int speed) {
	if (_GP(game).color_depth > 1) {
		set_palette(p);

		_GP(play).screen_is_faded_out = 0;

		if (_GP(play).no_hicolor_fadein) {
			return;
		}
	}

	gfxDriver->FadeIn(speed, p, _GP(play).fade_to_red, _GP(play).fade_to_green, _GP(play).fade_to_blue);
}

Bitmap *saved_viewport_bitmap = nullptr;
color old_palette[256];
void current_fade_out_effect() {
	if (pl_run_plugin_hooks(AGSE_TRANSITIONOUT, 0))
		return;

	// get the screen transition type
	int theTransition = _GP(play).fade_effect;
	// was a temporary transition selected? if so, use it
	if (_GP(play).next_screen_transition >= 0)
		theTransition = _GP(play).next_screen_transition;
	const bool ignore_transition = _GP(play).screen_tint > 0;

	if ((theTransition == FADE_INSTANT) || ignore_transition) {
		if (!_GP(play).keep_screen_during_instant_transition)
			set_palette_range(_G(black_palette), 0, 255, 0);
	} else if (theTransition == FADE_NORMAL) {
		my_fade_out(5);
	} else if (theTransition == FADE_BOXOUT) {
		gfxDriver->BoxOutEffect(true, get_fixed_pixel_size(16), 1000 / GetGameSpeed());
		_GP(play).screen_is_faded_out = 1;
	} else {
		get_palette(old_palette);
		const Rect &viewport = _GP(play).GetMainViewport();
		saved_viewport_bitmap = CopyScreenIntoBitmap(viewport.GetWidth(), viewport.GetHeight());
	}
}

IDriverDependantBitmap *prepare_screen_for_transition_in() {
	if (saved_viewport_bitmap == nullptr)
		quit("Crossfade: buffer is null attempting transition");

	saved_viewport_bitmap = ReplaceBitmapWithSupportedFormat(saved_viewport_bitmap);
	const Rect &viewport = _GP(play).GetMainViewport();
	if (saved_viewport_bitmap->GetHeight() < viewport.GetHeight()) {
		Bitmap *enlargedBuffer = BitmapHelper::CreateBitmap(saved_viewport_bitmap->GetWidth(), viewport.GetHeight(), saved_viewport_bitmap->GetColorDepth());
		enlargedBuffer->Blit(saved_viewport_bitmap, 0, 0, 0, (viewport.GetHeight() - saved_viewport_bitmap->GetHeight()) / 2, saved_viewport_bitmap->GetWidth(), saved_viewport_bitmap->GetHeight());
		delete saved_viewport_bitmap;
		saved_viewport_bitmap = enlargedBuffer;
	} else if (saved_viewport_bitmap->GetHeight() > viewport.GetHeight()) {
		Bitmap *clippedBuffer = BitmapHelper::CreateBitmap(saved_viewport_bitmap->GetWidth(), viewport.GetHeight(), saved_viewport_bitmap->GetColorDepth());
		clippedBuffer->Blit(saved_viewport_bitmap, 0, (saved_viewport_bitmap->GetHeight() - viewport.GetHeight()) / 2, 0, 0, saved_viewport_bitmap->GetWidth(), saved_viewport_bitmap->GetHeight());
		delete saved_viewport_bitmap;
		saved_viewport_bitmap = clippedBuffer;
	}
	IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(saved_viewport_bitmap, false);
	return ddb;
}

//=============================================================================
//
// Screen script API.
//
//=============================================================================

int Screen_GetScreenWidth() {
	return _GP(game).GetGameRes().Width;
}

int Screen_GetScreenHeight() {
	return _GP(game).GetGameRes().Height;
}

bool Screen_GetAutoSizeViewport() {
	return _GP(play).IsAutoRoomViewport();
}

void Screen_SetAutoSizeViewport(bool on) {
	_GP(play).SetAutoRoomViewport(on);
}

ScriptViewport *Screen_GetViewport() {
	return _GP(play).GetScriptViewport(0);
}

int Screen_GetViewportCount() {
	return _GP(play).GetRoomViewportCount();
}

ScriptViewport *Screen_GetAnyViewport(int index) {
	return _GP(play).GetScriptViewport(index);
}

ScriptUserObject *Screen_ScreenToRoomPoint(int scrx, int scry) {
	data_to_game_coords(&scrx, &scry);

	VpPoint vpt = _GP(play).ScreenToRoom(scrx, scry);
	if (vpt.second < 0)
		return nullptr;

	game_to_data_coords(vpt.first.X, vpt.first.Y);
	return ScriptStructHelpers::CreatePoint(vpt.first.X, vpt.first.Y);
}

ScriptUserObject *Screen_RoomToScreenPoint(int roomx, int roomy) {
	data_to_game_coords(&roomx, &roomy);
	Point pt = _GP(play).RoomToScreen(roomx, roomy);
	game_to_data_coords(pt.X, pt.Y);
	return ScriptStructHelpers::CreatePoint(pt.X, pt.Y);
}

RuntimeScriptValue Sc_Screen_GetScreenHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Screen_GetScreenHeight);
}

RuntimeScriptValue Sc_Screen_GetScreenWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Screen_GetScreenWidth);
}

RuntimeScriptValue Sc_Screen_GetAutoSizeViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL(Screen_GetAutoSizeViewport);
}

RuntimeScriptValue Sc_Screen_SetAutoSizeViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PBOOL(Screen_SetAutoSizeViewport);
}

RuntimeScriptValue Sc_Screen_GetViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptViewport, Screen_GetViewport);
}

RuntimeScriptValue Sc_Screen_GetViewportCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Screen_GetViewportCount);
}

RuntimeScriptValue Sc_Screen_GetAnyViewport(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT(ScriptViewport, Screen_GetAnyViewport);
}

RuntimeScriptValue Sc_Screen_ScreenToRoomPoint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT2(ScriptUserObject, Screen_ScreenToRoomPoint);
}

RuntimeScriptValue Sc_Screen_RoomToScreenPoint(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_PINT2(ScriptUserObject, Screen_RoomToScreenPoint);
}

void RegisterScreenAPI() {
	ccAddExternalStaticFunction("Screen::get_Height", Sc_Screen_GetScreenHeight);
	ccAddExternalStaticFunction("Screen::get_Width", Sc_Screen_GetScreenWidth);
	ccAddExternalStaticFunction("Screen::get_AutoSizeViewportOnRoomLoad", Sc_Screen_GetAutoSizeViewport);
	ccAddExternalStaticFunction("Screen::set_AutoSizeViewportOnRoomLoad", Sc_Screen_SetAutoSizeViewport);
	ccAddExternalStaticFunction("Screen::get_Viewport", Sc_Screen_GetViewport);
	ccAddExternalStaticFunction("Screen::get_ViewportCount", Sc_Screen_GetViewportCount);
	ccAddExternalStaticFunction("Screen::geti_Viewports", Sc_Screen_GetAnyViewport);
	ccAddExternalStaticFunction("Screen::ScreenToRoomPoint", Sc_Screen_ScreenToRoomPoint);
	ccAddExternalStaticFunction("Screen::RoomToScreenPoint", Sc_Screen_RoomToScreenPoint);
}

} // namespace AGS3
