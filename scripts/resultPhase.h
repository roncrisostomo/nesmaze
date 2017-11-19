/******************************************************************************
*  @file       	resultPhase.h
*  @brief      	Result phase handler
*  @author     	Ron
*  @created 	November 17, 2017
*  @modified   	November 18, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the result phase
******************************************************************************/

#include "nametables/result_success.h"
#include "nametables/result_failure.h"

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
