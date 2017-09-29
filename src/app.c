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

#define COLOR_ROWS_COUNT 3
#define RESERVED_ROWS_COUNT (PAD_ROWS_COUNT - COLOR_ROWS_COUNT)

#define BOTTOM_BUTTONS_COUNT (BOTTOM_ROWS_COUNT * BUTTONS_PER_ROW)
#define COLOR_BUTTONS_COUNT (COLOR_ROWS_COUNT * BUTTONS_PER_ROW)
#define RESERVED_BUTTONS_COUNT (RESERVED_ROWS_COUNT * BUTTONS_PER_ROW)
#define TOP_BUTTONS_COUNT (TOP_ROWS_COUNT * BUTTONS_PER_ROW)

// static buttons indizes
#define RESET_BUTTON_INDEX_X 0
#define SEND_BUTTON_INDEX_X 9

// variables

// colors    wht red grn blu amb wht cya mag yel wht
u8 red[] = { 63, 63,  0,  0, 63, 63,  0, 63,  0, 63 };
u8 grn[] = { 63,  0, 63,  0, 31, 63, 63,  0, 63, 63 };
u8 blu[] = { 63,  0,  0, 63,  0, 63, 63, 63, 63, 63 };

u8 preSelectedIndexX[PAD_ROWS_COUNT];
u8 selectedIndexX[PAD_ROWS_COUNT];

void app_surface_event(u8 type, u8 index, u8 value)
{
	switch (type)
	{
		case  TYPEPAD:
		{
			// bottom row
			if (index <= BOTTOM_BUTTONS_COUNT)
			{
				// ignore index 0 and 9
			}
			
			// color rows
			else if (index <= BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT)
			{
				// foreach color row
				for (int i=1; i<=COLOR_ROWS_COUNT; i++)
				{
					u8 indexX = index % BUTTONS_PER_ROW; 
					u8 indexY = (index - indexX) / BUTTONS_PER_ROW;

					switch(indexX)
					{
						case RESET_BUTTON_INDEX_X:
							// logic
							for (int j=1; j<BUTTONS_PER_ROW - 1; j++)
							{
								u8 colorButtonIndex = indexY * BUTTONS_PER_ROW + j;
								hal_send_midi(DINMIDI, NOTEOFF, colorButtonIndex, 0);
							}
							preSelectedIndexX[indexY] = 0;
							
							// visualization
							for (int j=0; j<BUTTONS_PER_ROW; j++)
							{
								u8 colorButtonIndex = indexY * BUTTONS_PER_ROW + j;
								hal_plot_led(TYPEPAD, colorButtonIndex, red[j], grn[j], blu[j]);
							}
							break;

						case SEND_BUTTON_INDEX_X: ;
							// logic
							u8 colorButtonIndexX = preSelectedIndexX[indexY];
							
							if (colorButtonIndexX == 0)
								return;
							
							u8 colorButtonIndex = indexY * BUTTONS_PER_ROW + colorButtonIndexX;
							hal_send_midi(DINMIDI, NOTEON, colorButtonIndex, 127);
							
							// visualization
							hal_plot_led(TYPEPAD, index, red[colorButtonIndexX], grn[colorButtonIndexX], blu[colorButtonIndexX]);
							break;

						default:
							// logic
							preSelectedIndexX[indexY] = indexX;

							// visualization
							if(value > 0)
								hal_plot_led(TYPEPAD, index, value, value, value);
							else
								hal_plot_led(TYPEPAD, index, red[indexX], grn[indexX], blu[indexX]);
							break;
					}
				}
			}

			// reserved rows 
			else if (index <= BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT + RESERVED_BUTTONS_COUNT)
			{

			}

			// top row
			else if (index <= BOTTOM_BUTTONS_COUNT + COLOR_BUTTONS_COUNT + RESERVED_BUTTONS_COUNT + TOP_BUTTONS_COUNT)
			{
				// ignore index 90 and 99
			}
		}
		break;

		case TYPESETUP:
		{
			// example - light the setup LED
			hal_plot_led(TYPESETUP, 0, value, value, value);
		}
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

void app_sysex_event(u8 port, u8 * data, u16 count)
{
	// example - respond to UDI messages?
}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
    // example - send poly aftertouch to MIDI ports
	hal_send_midi(USBMIDI, POLYAFTERTOUCH | 0, index, value);

    // example - set LED to white, brightness in proportion to pressure
	hal_plot_led(TYPEPAD, index, value/2, value/2, value/2);
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
#define TICK_MS 20
	
	static u8 ms = TICK_MS;
	
	if (++ms >= TICK_MS)
	{

	}
}

//______________________________________________________________________________

void app_init()
{
	// example - light the LEDs to say hello!
	for (int i=0; i < 10; ++i)
	{
		for (int j=0; j < 10; ++j)
		{
			u8 r = i < 5 ? (MAXLED * (5-i))/5 : 0;
			u8 g = i < 5 ? (MAXLED * i)/5 : (MAXLED * (10-i))/5;
			u8 b = i < 5 ? 0 : (MAXLED * (i-5))/5;

			hal_plot_led(TYPEPAD, j*10 + i, r, b, g);
		}
	}
	
	for (int i=BOTTOM_BUTTONS_COUNT; i<=( TOP_BUTTONS_COUNT + COLOR_BUTTONS_COUNT ); i+=BUTTONS_PER_ROW)
	{
		u8 indexX = i % BUTTONS_PER_ROW;
		hal_plot_led(TYPEPAD, i, red[indexX], grn[indexX], blu[indexX]); 
	}
}
