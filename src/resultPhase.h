/******************************************************************************
*  @file       	resultPhase.h
*  @brief      	Result phase handler
*  @author     	Ron
*  @created 	November 17, 2017
*  @modified   	November 19, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the result phase
******************************************************************************/

#include "nametables/result_success.h"
#include "nametables/result_failure.h"

// Nametable position and length of "score" text
#define SCORE_TEXT_ADR 	(NTADR_A(13,18))

void resultPhase(void)
{
	// Result loop
	//	Load result according to success flag
	//	Fade in result screen
	//	On Start button press:
	//		Fade out result screen
	//		Exit result loop (return to main infinite loop -> title loop)
	
	// Load result nametable
	vram_adr(NAMETABLE_A);
	if (gameClear)
	{
		vram_unrle(result_success);
		
		// Write "score" text horizontally centered on screen
		if (totalItemsCollected5 > 0)
		{
			vram_adr(SCORE_TEXT_ADR);
			vram_put(0x10 + totalItemsCollected5);
			vram_put(0x10 + totalItemsCollected4);
			vram_put(0x10 + totalItemsCollected3);
			vram_put(0x10 + totalItemsCollected2);
			vram_put(0x10 + totalItemsCollected1);
		}
		else if (totalItemsCollected4 > 0)
		{
			vram_adr(SCORE_TEXT_ADR + 1);
			vram_put(0x10 + totalItemsCollected4);
			vram_put(0x10 + totalItemsCollected3);
			vram_put(0x10 + totalItemsCollected2);
			vram_put(0x10 + totalItemsCollected1);
		}
		else if (totalItemsCollected3 > 0)
		{
			vram_adr(SCORE_TEXT_ADR + 1);
			vram_put(0x10 + totalItemsCollected3);
			vram_put(0x10 + totalItemsCollected2);
			vram_put(0x10 + totalItemsCollected1);
		}
		else if (totalItemsCollected2 > 0)
		{
			vram_adr(SCORE_TEXT_ADR + 2);
			vram_put(0x10 + totalItemsCollected2);
			vram_put(0x10 + totalItemsCollected1);
		}
		else
		{
			vram_adr(SCORE_TEXT_ADR + 2);
			vram_put(0x10 + totalItemsCollected1);
		}
	}
	else
	{
		vram_unrle(result_failure);
	}
	
	// Enable BG
	ppu_on_bg();
	
	// Fade in result screen
	pal_fade_to(4);
	
	while (1)
	{
		ppu_wait_frame();
		
		// If start button is pressed, exit the result loop
		if (pad_trigger(0)&PAD_START)	break;
	}
	
	pal_fade_to(0);
}
