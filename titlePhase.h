/******************************************************************************
*  @file       	titlePhase.h
*  @brief      	Title phase handler
*  @author     	Ron
*  @created 	November 17, 2017
*  @modified   	November 17, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the title phase
******************************************************************************/

// Nametables for game screens
#include "title_nam.h"

// Index of color of "press start" text in title palette, used for blinking animation
#define PRESS_START_PAL_INDEX 6

// Palettes
const unsigned char palTitle[16] = { 0x0f,0x00,0x10,0x30,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

void titlePhase(void)
{
	// Start and target bright levels
	i = 0;
	j = 4;

	// Start faded out
	pal_bright(i);
	
	// Reset scroll
	scroll(0, 0);
	
	// Load title nametable
	vram_adr(NAMETABLE_A);
	vram_unrle(title_nam);
	// Set title palette
	pal_bg(palTitle);
	// Turn on background display
	ppu_on_bg();

	// Counter used for title screen fade-in
	frameCounter = 0;
	
	while (1)
	{
		ppu_wait_frame();
		
		// If start button is pressed, exit the title loop
		if (pad_trigger(0)&PAD_START)	break;
		
		// Update frame count
		frameCounter++;

		// Update title screen fade in
		// Note: Not using pal_fade_to here to allow polling start button simultaneously
		if (i < j)
		{
			// Fade in title screen
			if (!(frameCounter&7))
			{
				i++;
				pal_bright(i);
			}
		}
		// Blink "press start" text
		else
		{
			// Toggle text color between BG color (black) and actual color (blue)
			pal_col(PRESS_START_PAL_INDEX, (frameCounter&32) ? 0x0f : 0x22);
		}
	}
		
	// Set bright level to target level
	pal_bright(j);
	
	// Play start game sound
	sfx_play(SFX_START, 0);
	
	// TODO: Uncomment
	/* // Blink text faster, 8x
	for (i = 0; i < 16; ++i)
	{
		pal_col(PRESS_START_PAL_INDEX, (i%2) ? 0x0f : 0x22);
		delay(4);
	} */

	// Fade out
	pal_fade_to(0);
}
