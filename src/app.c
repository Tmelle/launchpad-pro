/******************************************************************************
 
 Copyright (c) 2015, Focusrite Audio Engineering Ltd.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of Focusrite Audio Engineering Ltd., nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 *****************************************************************************/

//______________________________________________________________________________
//
// Headers
//______________________________________________________________________________

#include "app.h"

//______________________________________________________________________________
//
// This is where the fun is!  Add your code to the callbacks below to define how
// your app behaves.
//______________________________________________________________________________

// defines

// macros
#define BETWEEN(value, min, max) (value < max && value > min)
#define BETWEEN_OR_EQ(value, min, max) (value <= max && value >= min)

#define ACTIVE 1.0
#define INACTIVE 0.1

#define RED 1
#define GREEN 2
#define BLUE 3
#define WHITE 4
#define AMBER 5
#define CYAN 6
#define MAGENTA 7
#define YELLOW 8

#define BUTTONS_PER_ROW 10

#define BOTTOM_ROWS_COUNT 1
#define PAD_ROWS_COUNT 8
#define TOP_ROWS_COUNT 1
#define ROWS_COUNT (BOTTOM_ROWS_COUNT + PAD_ROWS_COUNT + TOP_ROWS_COUNT)

#define COLOR_ROWS_COUNT 3
#define GOBO_ROWS_COUNT 3
#define FOCUS_ROWS_COUNT 1
#define POSITION_ROWS_COUNT 1
#define SELECT_ROWS_COUNT (COLOR_ROWS_COUNT + GOBO_ROWS_COUNT + POSITION_ROWS_COUNT + FOCUS_ROWS_COUNT)

#define BOTTOM_BUTTONS_COUNT (BOTTOM_ROWS_COUNT * BUTTONS_PER_ROW)
#define COLOR_BUTTONS_COUNT (COLOR_ROWS_COUNT * BUTTONS_PER_ROW)
#define GOBO_BUTTONS_COUNT (GOBO_ROWS_COUNT * BUTTONS_PER_ROW)
#define FOCUS_BUTTONS_COUNT (FOCUS_ROWS_COUNT * BUTTONS_PER_ROW)
#define POSITION_BUTTONS_COUNT (POSITION_ROWS_COUNT * BUTTONS_PER_ROW)
#define SELECT_BUTTONS_COUNT (SELECT_ROWS_COUNT * BUTTONS_PER_ROW)
#define TOP_BUTTONS_COUNT (TOP_ROWS_COUNT * BUTTONS_PER_ROW)

// static buttons indizes
#define SEND_ALL_BUTTON_INDEX 8
#define RESET_ALL_BUTTON_INDEX 1

#define RESET_BUTTON_INDEX_X 0
#define SEND_BUTTON_INDEX_X 9

#define SELECTIONS_PER_ROW 2
#define DEFAULT_SELECTION 0

// variables

// colors    wht red grn blu amb wht cya mag yel wht
u8 red[] = {63, 63, 0, 0, 63, 63, 0, 63, 63, 63};
u8 grn[] = {63, 0, 63, 0, 31, 63, 63, 0, 63, 63};
u8 blu[] = {63, 0, 0, 63, 0, 63, 63, 63, 0, 63};

u8 colorMapRed[ROWS_COUNT * BUTTONS_PER_ROW];
u8 colorMapGrn[ROWS_COUNT * BUTTONS_PER_ROW];
u8 colorMapBlu[ROWS_COUNT * BUTTONS_PER_ROW];

u8 preSelectedIndexX[ROWS_COUNT][SELECTIONS_PER_ROW];
u8 selectedIndexX[ROWS_COUNT][SELECTIONS_PER_ROW];

u8 selectionsCount[ROWS_COUNT] = {0, 1, 1, 1, 1, 1, 1, 2, 2, 0};

void app_surface_event(u8 type, u8 index, u8 value)
{
	switch (type)
	{
	case TYPEPAD:
		app_handle_button_press(index, value);
		break;

	case TYPESETUP:
		// example - light the setup LED
		hal_plot_led(TYPESETUP, 0, value, value, value);
		break;
	}
}

//______________________________________________________________________________

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
	// example - MIDI interface functionality for USB "MIDI" port -> DIN port
	if (port == USBMIDI)
	{
		hal_send_midi(DINMIDI, status, d1, d2);
	}

	// // example -MIDI interface functionality for DIN -> USB "MIDI" port port
	if (port == DINMIDI)
	{
		hal_send_midi(USBMIDI, status, d1, d2);
	}
}

//______________________________________________________________________________

void app_sysex_event(u8 port, u8 *data, u16 count)
{
}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
}

//______________________________________________________________________________

void app_cable_event(u8 type, u8 value)
{
	// example - light the Setup LED to indicate cable connections
	if (type == MIDI_IN_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, 0, value, 0); // green
	}
	else if (type == MIDI_OUT_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, value, 0, 0); // red
	}
}

//______________________________________________________________________________

void app_timer_event()
{
// example - send MIDI clock at 125bpm
#define TICK_MS_ON 0
#define TICK_MS_OFF 500
#define TICK_RESET 1000

	static u16 ms = 0;

	if (ms == TICK_MS_ON || ms == TICK_MS_OFF)
	{
		float intensity = ms == TICK_MS_ON ? ACTIVE : INACTIVE;
		for (int indexY = 1; indexY <= PAD_ROWS_COUNT; indexY++)
		{
			for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
			{
				u8 indexX = preSelectedIndexX[indexY][selection];
				if (indexX == 0)
					continue;
				u8 index = indexY * BUTTONS_PER_ROW + indexX;
				hal_plot_led(TYPEPAD, index, colorMapRed[index] * intensity, colorMapGrn[index] * intensity, colorMapBlu[index] * intensity);
			}
		}
	}

	ms++;
	if (ms >= TICK_RESET)
		ms = 0;
}

//______________________________________________________________________________

void app_init()
{
	for (int indexY = 0; indexY < ROWS_COUNT; indexY++)
	{
		selectedIndexX[indexY][DEFAULT_SELECTION] = 0;
		preSelectedIndexX[indexY][DEFAULT_SELECTION] = 0;
	}

	app_init_color_map();
	app_init_leds();
}

void app_init_color_map()
{
	// init color map
	for (int index = 0; index < (ROWS_COUNT * BUTTONS_PER_ROW); index++)
	{
		u8 indexX = index % BUTTONS_PER_ROW;
		u8 indexY = (index - indexX) / BUTTONS_PER_ROW;

		if (index < BOTTOM_BUTTONS_COUNT)
		{
			colorMapRed[index] = index == SEND_ALL_BUTTON_INDEX || index == RESET_ALL_BUTTON_INDEX ? 63 : 0;
			colorMapGrn[index] = index == SEND_ALL_BUTTON_INDEX || index == RESET_ALL_BUTTON_INDEX ? 63 : 0;
			colorMapBlu[index] = index == SEND_ALL_BUTTON_INDEX || index == RESET_ALL_BUTTON_INDEX ? 63 : 0;
		}
		else if (index < BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT)
		{
			colorMapRed[index] = red[indexX];
			colorMapGrn[index] = grn[indexX];
			colorMapBlu[index] = blu[indexX];
		}
		else if (index < BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT + GOBO_BUTTONS_COUNT)
		{
			if (indexX != RESET_BUTTON_INDEX_X && indexX != SEND_BUTTON_INDEX_X)
			{
				colorMapRed[index] = 0;
				colorMapGrn[index] = 63;
				colorMapBlu[index] = 63;
			}
			else
			{
				colorMapRed[index] = 63;
				colorMapGrn[index] = 63;
				colorMapBlu[index] = 63;
			}
		}
		else if (index < BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT + GOBO_BUTTONS_COUNT + FOCUS_BUTTONS_COUNT)
		{
			if (indexX != RESET_BUTTON_INDEX_X && indexX != SEND_BUTTON_INDEX_X)
			{
				colorMapRed[index] = 0;
				colorMapGrn[index] = 63;
				colorMapBlu[index] = 31;
			}
			else
			{
				colorMapRed[index] = 63;
				colorMapGrn[index] = 63;
				colorMapBlu[index] = 63;
			}
		}
		else if (index < BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT + GOBO_BUTTONS_COUNT + FOCUS_BUTTONS_COUNT + POSITION_BUTTONS_COUNT)
		{
			if (indexX != RESET_BUTTON_INDEX_X && indexX != SEND_BUTTON_INDEX_X)
			{
				colorMapRed[index] = 63;
				colorMapGrn[index] = 0;
				colorMapBlu[index] = 0;
			}
			else
			{
				colorMapRed[index] = 63;
				colorMapGrn[index] = 63;
				colorMapBlu[index] = 63;
			}
		}
		else if (index < BOTTOM_BUTTONS_COUNT + SELECT_BUTTONS_COUNT + TOP_BUTTONS_COUNT)
		{
			colorMapRed[index] = 0;
			colorMapGrn[index] = 0;
			colorMapBlu[index] = 0;
		}
	}
}

void app_init_leds()
{
	for (int index = 0; index < (ROWS_COUNT * BUTTONS_PER_ROW); index++)
	{
		hal_plot_led(TYPEPAD, index, colorMapRed[index] * INACTIVE, colorMapGrn[index] * INACTIVE, colorMapBlu[index] * INACTIVE);
	}
}

void app_reset_selected_index(u8 indexY, u8 selection)
{
	// logic
	u8 oldSelectedIndexX = selectedIndexX[indexY][selection];
	u8 oldSelectedIndex = indexY * BUTTONS_PER_ROW + oldSelectedIndexX;

	u8 sendButtonIndexX = SEND_BUTTON_INDEX_X;
	u8 sendButtonIndex = indexY * BUTTONS_PER_ROW + sendButtonIndexX;

	selectedIndexX[indexY][selection] = 0;

	// visualization
	if (oldSelectedIndex != 0)
		hal_plot_led(TYPEPAD, oldSelectedIndex, colorMapRed[oldSelectedIndex] * INACTIVE, colorMapGrn[oldSelectedIndex] * INACTIVE, colorMapBlu[oldSelectedIndex] * INACTIVE);

	hal_plot_led(TYPEPAD, sendButtonIndex, colorMapRed[sendButtonIndex] * INACTIVE, colorMapGrn[sendButtonIndex] * INACTIVE, colorMapBlu[sendButtonIndex] * INACTIVE);
		

	// midi
	if (oldSelectedIndex != 0)
		hal_send_midi(DINMIDI, NOTEOFF, oldSelectedIndex, 0);
}

void app_set_selected_index(u8 indexX, u8 indexY, u8 selection)
{
	// logic
	app_reset_selected_index(indexY, selection);

	u8 newSelectedIndexX = indexX;
	u8 newSelectedIndex = indexY * BUTTONS_PER_ROW + newSelectedIndexX;

	u8 sendButtonIndexX = SEND_BUTTON_INDEX_X;
	u8 sendButtonIndex = indexY * BUTTONS_PER_ROW + sendButtonIndexX;

	selectedIndexX[indexY][selection] = newSelectedIndexX;

	if (newSelectedIndexX != 0)
	{
		hal_plot_led(TYPEPAD, newSelectedIndex, colorMapRed[newSelectedIndex] * ACTIVE, colorMapGrn[newSelectedIndex] * ACTIVE, colorMapBlu[newSelectedIndex] * ACTIVE);
		hal_plot_led(TYPEPAD, sendButtonIndex, colorMapRed[newSelectedIndex] * ACTIVE, colorMapGrn[newSelectedIndex] * ACTIVE, colorMapBlu[newSelectedIndex] * ACTIVE);
	}

	// midi
	if (newSelectedIndex != 0)
		hal_send_midi(DINMIDI, NOTEON, newSelectedIndex, 127);
}

void app_reset_pre_selected_index(u8 indexY, u8 selection)
{
	// logic
	u8 oldPreSelectedIndexX = preSelectedIndexX[indexY][selection];
	u8 oldPreSelectedIndex = indexY * BUTTONS_PER_ROW + oldPreSelectedIndexX;

	preSelectedIndexX[indexY][selection] = 0;

	// visualization
	if (oldPreSelectedIndexX != 0 && oldPreSelectedIndexX != selectedIndexX[indexY][selection])
		hal_plot_led(TYPEPAD, oldPreSelectedIndex, colorMapRed[oldPreSelectedIndex] * INACTIVE, colorMapGrn[oldPreSelectedIndex] * INACTIVE, colorMapBlu[oldPreSelectedIndex] * INACTIVE);
}

void app_set_pre_selected_index(u8 indexX, u8 indexY, u8 selection)
{
	// logic
	app_reset_pre_selected_index(indexY, selection);

	u8 newPreSelectedIndexX = indexX;
	u8 newPreSelectedIndex = indexY * BUTTONS_PER_ROW + newPreSelectedIndexX;

	preSelectedIndexX[indexY][selection] = newPreSelectedIndexX;
}

u8 app_get_selection(u8 indexX, u8 indexY)
{
	// TODO: use constants 
	u8 buttonsPerSelection = 8 / selectionsCount[indexY];
	for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
	{
		if (indexX > ((selection + 1) * buttonsPerSelection))
			continue;
		return selection;
	}
}

// events

void app_handle_button_press(u8 index, u8 value)
{
	u8 indexX = index % BUTTONS_PER_ROW;
	u8 indexY = (index - indexX) / BUTTONS_PER_ROW;

	// bottom row
	if (index < BOTTOM_BUTTONS_COUNT)
		app_handle_bottom_button_press(index, indexX, indexY, value);

	// select rows
	else if (index < BOTTOM_BUTTONS_COUNT + SELECT_BUTTONS_COUNT)
		app_handle_selection_button_press(index, indexX, indexY, value);

	// top row
	else if (index < BOTTOM_BUTTONS_COUNT + SELECT_BUTTONS_COUNT + TOP_BUTTONS_COUNT)
		app_handle_top_button_press(index, indexX, indexY, value);
}

void app_handle_bottom_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	switch (index)
	{
	case SEND_ALL_BUTTON_INDEX:
		app_handle_send_all_button_press(index, indexX, indexY, value);
		break;

	case RESET_ALL_BUTTON_INDEX:
		app_handle_reset_all_button_press(index, indexX, indexY, value);
		break;
	}
}

void app_handle_send_all_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	if (value == 0)
		return;

	for (int indexY = BOTTOM_ROWS_COUNT; indexY < BOTTOM_ROWS_COUNT + SELECT_ROWS_COUNT; indexY++)
	{
		for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
		{
			u8 newSelectedIndexX = preSelectedIndexX[indexY][selection];
			if (newSelectedIndexX == 0)
				continue;

			app_reset_pre_selected_index(indexY, selection);
			app_set_selected_index(newSelectedIndexX, indexY, selection);
		}
	}
}

void app_handle_reset_all_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	if (value == 0)
		return;

	for (int indexY = BOTTOM_ROWS_COUNT; indexY < BOTTOM_ROWS_COUNT + SELECT_ROWS_COUNT; indexY++)
	{
		for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
		{
			app_reset_pre_selected_index(indexY, selection);
			app_reset_selected_index(indexY, selection);
		}
	}
}

void app_handle_top_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
}

void app_handle_selection_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	switch (indexX)
	{
	case RESET_BUTTON_INDEX_X:;
		app_handle_reset_button_press(index, indexX, indexY, value);
		break;

	case SEND_BUTTON_INDEX_X:;
		app_handle_send_button_press(index, indexX, indexY, value);
		break;

	default:
		app_handle_value_button_press(index, indexX, indexY, value);
		break;
	}
}

void app_handle_value_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	// logic
	for (int selection = 0; selection < SELECTIONS_PER_ROW; selection++)
	{
		if (indexX == selectedIndexX[indexY][selection])
			return;
	}

	u8 selection = app_get_selection(indexX, indexY);

	if (value != 0)
		app_set_pre_selected_index(indexX, indexY, selection);

	// visualization
	float intensity = value != 0 ? ACTIVE : INACTIVE;
	hal_plot_led(TYPEPAD, index, colorMapRed[index] * intensity, colorMapGrn[index] * intensity, colorMapBlu[index] * intensity);
}

void app_handle_send_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	if (value == 0)
		return;

	for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
	{
		u8 newSelectedIndexX = preSelectedIndexX[indexY][selection];
		if (newSelectedIndexX == 0)
			continue;

		app_reset_pre_selected_index(indexY, selection);
		app_set_selected_index(newSelectedIndexX, indexY, selection);
	}
}
void app_handle_reset_button_press(u8 index, u8 indexX, u8 indexY, u8 value)
{
	if (value == 0)
		return;

	for (u8 selection = 0; selection < selectionsCount[indexY]; selection++)
	{
		app_reset_pre_selected_index(indexY, selection);
		app_reset_selected_index(indexY, selection);
	}
}