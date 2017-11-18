/******************************************************************************
*  @file       	gamePhase.h
*  @brief      	Game phase handler
*  @author     	Ron
*  @created 	November 17, 2017
*  @modified   	November 18, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the game phase
******************************************************************************/

// Game constants
#define START_SPEED 		2
#define SPEED_UP_PER_LEVEL	1
// Delay between display on and game start (minimum 1)
#define START_DELAY			10
// Delay between meeting win/lose condition and game screen fade out
#define END_DELAY			50

// Level map nametables
#include "level_test.h"
#include "level_test2.h"

// Level map palettes
const unsigned char pal_level_test[16] = { 0x0f,0x00,0x10,0x30,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

// Sprite palettes
const unsigned char palGameSpr[16] = { 0x0f,0x0f,0x29,0x30,0x0f,0x0f,0x26,0x30,0x0f,0x0f,0x24,0x30,0x0f,0x0f,0x21,0x30 };

// Max size of the game map (in number of map tiles)
#define MAP_WIDTH		16
#define MAP_WIDTH_BIT	4
#define MAP_HEIGHT		13

// Macro for calculating map offset from screen space, as
// the map size is smaller than screen to save some memory
#define MAP_ADR(x,y)	((((y)-2)<<MAP_WIDTH_BIT)|(x))

// Size of a map tile
#define TILE_SIZE		16		// Tiles are 16x16 bits
#define TILE_SIZE_BIT	4		// Movement within tile can be represented with 4 bits (2^4)

#define TILE_PLUS_FP_BITS	(TILE_SIZE_BIT + FP_BITS)

// Tile code legend, i.e. what codes in level maps mean
#define TILE_START		0x33	// 'S'
#define TILE_EXIT		0x25	// 'E'
#define TILE_WALL		0x40	// Upper-left corner of wall tile
#define TILE_EMPTY		0x44	// Empty tile
// TODO: Use this
#define TILE_HOLE		0x00	// Blank "lose condition" tile
#define TILE_ITEM		0x45	// Upper-left corner of item tile

// Address where maps begin on screen
#define MAP_START_ADR	0x80	// Start on row 3, below HUD row

// Nametable position and length of HUD labels
#define HUD_LABELS_POS 	0x44
#define HUD_LABELS_LEN	23

// HUD Labels
const unsigned char hudLabels[HUD_LABELS_LEN] = {
	0x2d,0x2f,0x2e,0x25,0x39,0x33,0x1a,0x10,0x10,0x10,0x10,0x10,0x00,0x23,0x2c,0x25,0x21,0x32,0x1a,0x10,0x10,0x10,0x05
};

// Metasprites (format: x offset, y offset, chr tile, palette)
const unsigned char playerMetasprite[] =
{
	0,-1,0x49,0,
	8,-1,0x4a,0,
	0, 7,0x4b,0,
	8, 7,0x4c,0,
	128
};

// Pre-initialized update list used during gameplay
#define UPDATE_LIST_SIZE (16*3+1)
const unsigned char updateListData[UPDATE_LIST_SIZE] =
{
	// "Empty" tile used to replace "item" tiles after their item has been collected
	0x28,0x00,TILE_EMPTY,
	0x28,0x00,TILE_EMPTY,
	0x28,0x00,TILE_EMPTY,
	0x28,0x00,TILE_EMPTY,
	// Blank tile used to replace tiles that player has just left
	0x28,0x00,TILE_HOLE,
	0x28,0x00,TILE_HOLE,
	0x28,0x00,TILE_HOLE,
	0x28,0x00,TILE_HOLE,
	// 5-digit HUD display for total collected items (all levels), initialized to "0"
	0x20,0x4b,0x10,
	0x20,0x4c,0x10,
	0x20,0x4d,0x10,
	0x20,0x4e,0x10,
	0x20,0x4f,0x10,
	// 3-digit HUD display for percent clear, initialized to "0"
	0x20,0x57,0x10,
	0x20,0x58,0x10,
	0x20,0x59,0x10,
	// End of file marker for VRAM update list
	NT_UPD_EOF
};

// Update list
static unsigned char updateList[UPDATE_LIST_SIZE];

// Array for game map, contains walls, empty spaces, and items
static unsigned char map[MAP_WIDTH*MAP_HEIGHT];

// This array is used to convert nametable into game map, row by row
static unsigned char nameRow[32];

// Number of items on current level
static unsigned char levelItemsCount;
// Number of items collected in current level
static unsigned char levelItemsCollected;
// Percent of items collected in current level
static unsigned char percentCollected;
// Total number of collected items
static unsigned char totalItemsCollected5;	// x0000
static unsigned char totalItemsCollected4;	// 0x000
static unsigned char totalItemsCollected3;	// 00x00
static unsigned char totalItemsCollected2;	// 000x0
static unsigned char totalItemsCollected1;	// 0000x

// Player variables
static unsigned int player_x;
static unsigned int player_y;
static unsigned char player_nextTileX;
static unsigned char player_nextTileY;
static unsigned char player_prevTileX;
static unsigned char player_prevTileY;
static unsigned char player_dir;
static unsigned char player_nextDir;
static int player_moveCounter;
static unsigned int player_speed;

// Exit position
static unsigned int exit_x;
static unsigned int exit_y;

// Whether current game level is done
static unsigned char levelDone;

// Increments the total number of items collected by 1
void incrementTotalItemsCollected(void)
{
	totalItemsCollected1++;
	if (totalItemsCollected1 == 10)
	{
		totalItemsCollected2++;
		totalItemsCollected1 = 0;
	}
	if (totalItemsCollected2 == 10)
	{
		totalItemsCollected3++;
		totalItemsCollected2 = 0;
	}
	if (totalItemsCollected3 == 10)
	{
		totalItemsCollected4++;
		totalItemsCollected3 = 0;
	}
	if (totalItemsCollected4 == 10)
	{
		totalItemsCollected5++;
		totalItemsCollected4 = 0;
	}
	if (totalItemsCollected5 == 10)
	{
		totalItemsCollected1 = 9;
		totalItemsCollected2 = 9;
		totalItemsCollected3 = 9;
		totalItemsCollected4 = 9;
		totalItemsCollected5 = 9;
	}
}

// Updates HUD values through the update list
void updateHUD(void)
{
	// Total collected in game
	updateList[26] = 0x10 + totalItemsCollected5;
	updateList[29] = 0x10 + totalItemsCollected4;
	updateList[32] = 0x10 + totalItemsCollected3;
	updateList[35] = 0x10 + totalItemsCollected2;
	updateList[38] = 0x10 + totalItemsCollected1;
	// Percent collected in current level
	updateList[41] = 0x10 + percentCollected/100;
	updateList[44] = 0x10 + percentCollected/10%10;
	updateList[47] = 0x10 + percentCollected%10;
}

// Checks whether player can move in the specified direction,
//	and updates player move variables if so
void checkPlayerMove(unsigned char dir)
{
	px = player_x >> TILE_PLUS_FP_BITS;
	py = player_y >> TILE_PLUS_FP_BITS;
	
	// Prevent moving off the left screen edge
	// TODO: Need checks for other edges?
	if (px == 0 && dir == DIR_LEFT)	return;
	
	switch (dir)
	{
	case DIR_LEFT:	--px;	break;
	case DIR_RIGHT:	++px;	break;
	case DIR_UP:	--py;	break;
	case DIR_DOWN:	++py;	break;
	}
	
	if (map[MAP_ADR(px,py)] == TILE_WALL)	return;
	
	player_nextTileX = px;
	player_nextTileY = py;
	
	player_moveCounter = TILE_SIZE << FP_BITS;
	player_dir = dir;
	player_nextDir = dir;
}

// Initializes the game screen, loading the HUD and parsing map data from the nametable for the current level
void initGameMap(void)
{
	// Clear sprites
	oam_clear();
	
	// Unpack level nametable into VRAM
	vram_adr(NAMETABLE_A);
	// TODO: Improve!
	vram_unrle(gameLevel == 0 ? level_test2 : level_test);
	
	// Load HUD
	vram_adr(NAMETABLE_A + HUD_LABELS_POS);
	vram_write((unsigned char*)hudLabels, HUD_LABELS_LEN);
	
	// Set level map palette
	pal_bg(pal_level_test);
	
	// Set sprite palette
	pal_spr(palGameSpr);
	
	// Read level nametable from VRAM
	i16 = NAMETABLE_A + MAP_START_ADR;
	ptr = 0;
	wait = 0;
	levelItemsCount = 0;
	levelItemsCollected = 0;
	percentCollected = 0;
	
	// If first level, reset totalItemsCollected
	if (gameLevel == 0)
	{
		totalItemsCollected1 = 0;
		totalItemsCollected2 = 0;
		totalItemsCollected3 = 0;
		totalItemsCollected4 = 0;
		totalItemsCollected5 = 0;
	}
	
	// TODO: Explain
	for (i = 2; i < MAP_HEIGHT+2; ++i)
	{
		// Read map data one row at a time (32 bytes) into nameRow
		vram_adr(i16);
		vram_read(nameRow, 32);
		// Reset address to start of row (advanced by read operation)
		vram_adr(i16);
		
		// Each map tile is 2 bytes wide, so read map data at increments of 2 up to the maximum map width (bytes = tiles*2)
		for (j = 0; j < MAP_WIDTH<<1; j += 2)
		{
			spr = nameRow[j];
			
			// TODO
			switch (spr)
			{
			case TILE_START:
				// TODO: Explain 3 and 4 bit shift!
				player_x = (j<<3)<<FP_BITS;
				player_y = (i<<4)<<FP_BITS;
				player_prevTileX = player_x;
				player_prevTileY = player_y;
				player_dir = DIR_NONE;
				player_nextDir = DIR_NONE;
				player_moveCounter = 0;
				// Speed increases every level
				player_speed = (START_SPEED + SPEED_UP_PER_LEVEL*gameLevel)<<FP_BITS;
				// Remove start marker from map
				spr = TILE_EMPTY;
				break;
			case TILE_EXIT:
				// Save exit position
				exit_x = (j<<3)<<FP_BITS;
				exit_y = (i<<4)<<FP_BITS;
				break;
			// case TILE_ENEMY:
			case TILE_ITEM:
				++levelItemsCount;
				break;
			}
			
			// Keep local copy of map data for use in collision detection etc
			map[ptr++] = spr;
			
			// Write back to VRAM
			vram_put(spr);
			// Keep 2nd byte in tile as is
			vram_put(nameRow[j+1]);
		}
		
		// Move to next tile row
		i16 += 64;	// 32 * 2, with 32 being offset for byte row, and 2 being tile height (2 bytes)
	}
	
	// Set up update list
	memcpy(updateList, updateListData, sizeof(updateListData));
	set_vram_update(updateList);
}

void gamePhase(void)
{			
	// Game loop
	//	Initialize:
	// 		Setup vram update for blank tiles and HUD coins
	// 		Load level nametable, HUD nametable
	//		Read vram, remove map start and exit points from map data
	//		Write back edited map to vram
	//		Spawn player sprite on map start point
	// 		Fade in game screen
	//	Loop:
	//		Detect input
	//		If arrows, move player in arrow direction
	//			Detect collisions with walls
	//			Make player movement tile-based (one press to automatically move one tile)
	//			When player reaches exit point, set success flag, exit game loop
	//		If buttons, play music/sfx
	//	Fade out game screen
	
	// Start with screen faded out
	pal_bright(0);
	
	// Initializes the game map, and takes info to be used for sprite placement
	initGameMap();
	
	// Short delay before starting game (animations, input processing)
	wait = START_DELAY;
	
	// Enable display
	ppu_on_all();
	
	// Update HUD values that carry over from previous level
	if (gameLevel > 0)
	{
		updateHUD();
	}
	
	levelDone = FALSE;
	frameCounter = 0;
	
	while (1)
	{
		// Set up sprites in OAM
		oam_meta_spr(player_x >> FP_BITS,
					 player_y >> FP_BITS,
					 0,
					 playerMetasprite);
		
		// Exit the loop after the metasprite update to make sure objects are at their final state
		if (levelDone)	break;
		
		// Wait for next frame
		// Note: OAM update must go first to make sure display is updated soon after any object changes
		ppu_wait_frame();
		
		++frameCounter;
		
		// Slowly fade brightness to needed value (max for gameplay, half for pause)
		if (!(frameCounter&3))
		{
			if (!gamePaused && bright < 4) ++bright;
			if ( gamePaused && bright > 2) --bright;
			pal_bright(bright);
		}
		
		// Check input in trigger mode
		input = pad_trigger(0);
		
		// Start button toggles pause mode
		if (input&PAD_START)
		{
			gamePaused ^= TRUE;
			music_pause(gamePaused);
		}
		
		// Don't process anything if game is paused
		if (gamePaused)	continue;
		
		// Animate BG and sprites via CHR bank switching
		bank_bg((frameCounter >> 4)&1);
		bank_spr((frameCounter >> 3)&1);	// Faster switching for sprites
		
		if (wait)
		{
			--wait;
			if (!wait)
			{
				// Play game music when wait time ends
				music_play(MUSIC_GAME);
			}
		}
		
		// If movement counter is not zero, process movement
		if (player_moveCounter != 0)
		{
			// Update position based on move direction
			switch (player_dir)
			{
				case DIR_RIGHT:	player_x += player_speed;	break;
				case DIR_LEFT:	player_x -= player_speed;	break;
				case DIR_DOWN:	player_y += player_speed;	break;
				case DIR_UP:	player_y -= player_speed;	break;
			}
			// Update movement counter
			player_moveCounter -= player_speed;
			
			// Snap position to tile coordinates
			if (player_moveCounter <= 0)
			{
				player_moveCounter = 0;
				// Keep only the tile coordinate bits, taking the "floor" of tile position
				/* player_x >>= TILE_PLUS_FP_BITS;
				player_y >>= TILE_PLUS_FP_BITS;
				// If moving up or left (negative offset relative to target tile position), it should be "ceiling" of tile position instead, i.e. "floor" + 1
				if (player_dir == DIR_LEFT)	player_x += 1;
				if (player_dir == DIR_UP)	player_y += 1;
				// Restore position variable format
				player_x <<= TILE_PLUS_FP_BITS;
				player_y <<= TILE_PLUS_FP_BITS; */
				player_x = player_nextTileX << TILE_PLUS_FP_BITS;
				player_y = player_nextTileY << TILE_PLUS_FP_BITS;
				
				// Check win condition: player reaches exit position
				if (player_x == exit_x && player_y == exit_y)
				{
					gameClear = TRUE;
					levelDone = TRUE;
				}
				
				i16 = MAP_ADR(player_nextTileX, player_nextTileY);
				switch (map[i16])
				{
				// Check lose condition: player moves to blank tile
				case TILE_HOLE:
					gameClear = FALSE;
					levelDone = TRUE;
					break;
				
				// Check for item pickups
				case TILE_ITEM:
					// Mark as collected in game map
					map[i16] = TILE_EMPTY;
					// Play item collect SFX
					// TODO: Why channel 2?
					sfx_play(SFX_ITEM, 2);
					// Update level collect count
					++levelItemsCollected;
					
					// Get percent of items collected in current level
					percentCollected = levelItemsCollected*100/levelItemsCount;
					
					// Update total collect count
					incrementTotalItemsCollected();
					
					// Get address of tile in nametable
					// TODO: Why << 6? Explain more!
					i16 = NAMETABLE_A + MAP_START_ADR +
						  ((player_nextTileY - 2) << 6) | (player_nextTileX << 1);
					
					// Replace with empty tile using the update list
					// Upper left subsprite
					updateList[0] = i16 >> 8;		// MSB
					updateList[1] = i16 & 0xff;		// LSB
					// Upper right subsprite
					updateList[3] = updateList[0];
					updateList[4] = updateList[1] + 1;	// Element next to upper left
					i16 += 32;
					// Lower left subsprite
					updateList[6] = i16 >> 8;
					updateList[7] = i16 & 0xff;
					// Lower right subsprite
					updateList[9] = updateList[6];
					updateList[10] = updateList[7] + 1;	// Element next to lower left
					
					// Update HUD
					updateHUD();
					break;
				}
				
				// Replace the previous tile with a blank tile
				i16 = MAP_ADR(player_prevTileX, player_prevTileY);
				map[i16] = TILE_HOLE;
				
				// Get address of previous tile in nametable
				i16 = NAMETABLE_A + MAP_START_ADR +
						  ((player_prevTileY - 2) << 6) | (player_prevTileX << 1);
				
				// Upper left subsprite
				updateList[12] = i16 >> 8;				// MSB
				updateList[13] = i16 & 0xff;			// LSB
				// Upper right subsprite
				updateList[15] = updateList[12];
				updateList[16] = updateList[13] + 1;	// Element next to upper left
				i16 += 32;
				// Lower left subsprite
				updateList[18] = i16 >> 8;
				updateList[19] = i16 & 0xff;
				// Lower right subsprite
				updateList[21] = updateList[18];
				updateList[22] = updateList[19] + 1;	// Element next to lower left
				
				// Set current player pos as "previous" pos for the next frame
				player_prevTileX = player_nextTileX;
				player_prevTileY = player_nextTileY;
				
				// Keep player moving in same direction until hitting a wall or until another possible move direction is selected
				checkPlayerMove(player_nextDir);
			}
		}
		
		// Get input state (previously polled with pad_trigger)
		j = pad_state(0);
		
		// If no movement to process, check for new input
		if (player_moveCounter == 0)
		{
			if (j&PAD_LEFT)		checkPlayerMove(DIR_LEFT);
			if (j&PAD_RIGHT)	checkPlayerMove(DIR_RIGHT);
			if (j&PAD_UP)		checkPlayerMove(DIR_UP);
			if (j&PAD_DOWN)		checkPlayerMove(DIR_DOWN);
		}
		else
		{
			if (j&PAD_LEFT)		player_nextDir = DIR_LEFT;
			if (j&PAD_RIGHT)	player_nextDir = DIR_RIGHT;
			if (j&PAD_UP)		player_nextDir = DIR_UP;
			if (j&PAD_DOWN)		player_nextDir = DIR_DOWN;
		}
	}
	
	// Process result
	if (gameClear)
	{
		music_play(MUSIC_CLEAR);
		++gameLevel;
	}
	else
	{
		music_play(MUSIC_LOSE);
		gameDone = TRUE;
	}
	
	// Delay to emphasize result
	delay(END_DELAY);
	
	// Fade out game screen
	pal_fade_to(0);
}
