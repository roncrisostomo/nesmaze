/******************************************************************************
*  @file       	gamePhase.h
*  @brief      	Game phase handler
*  @author     	Ron
*  @created 	November 17, 2017
*  @modified   	November 24, 2017
*      
*  @par [explanation]
*		> Holds code used exclusively in the game phase
******************************************************************************/

// Level map nametables
#include "nametables/level_test.h"
#include "nametables/level_test2.h"
#include "nametables/level_test3.h"

// Level map palettes
const unsigned char pal_level_test[16] = { 0x0f,0x00,0x10,0x30,0x0f,0x01,0x21,0x31,0x0f,0x06,0x16,0x26,0x0f,0x09,0x19,0x29 };

// Sprite palettes
const unsigned char palGameSpr[16] = { 0x0f,0x0f,0x29,0x30,0x0f,0x0f,0x26,0x30,0x0f,0x0f,0x24,0x30,0x0f,0x0f,0x21,0x30 };

// Max size of the game map (in number of map tiles)
#define MAP_WIDTH		16
#define MAP_WIDTH_BIT	4
#define MAP_HEIGHT		13

// Number of rows occupied by the HUD at the top of the screen
#define HUD_HEIGHT		2

#define MAP_START_ADR	(NTADR_A(0, HUD_HEIGHT<<1))

// Macro for calculating nametable address from tile coordinates
// x and y are in tile coordinates
// Multiply both by 2 (<<1) to convert tiles to bytes (1 tile = 2 bytes)
// Multiply y by 32 (<<5) to get the starting row address (1 row = 32 bytes)
// Then add x (y'|x') for the column address
#define NAM_ADR(x,y)	(MAP_START_ADR + (((y)-HUD_HEIGHT)<<6) | ((x)<<1))
// Macro for calculating map offset from tile coordinates
// Explanation is similar to above, but with 0 start adr,
// different row width, and no need for conversion to bytes
#define MAP_ADR(x,y)	((((y)-HUD_HEIGHT)<<MAP_WIDTH_BIT) | (x))

// Size of a map tile
#define TILE_SIZE		16		// Tiles are 16x16 bits
#define TILE_SIZE_BIT	4		// Movement within tile can be represented with 4 bits (2^4)

// Used for converting position variables to tile coordinates
#define TILE_PLUS_FP_BITS	(TILE_SIZE_BIT + FP_BITS)

// Tile code legend, i.e. what codes in level maps mean
#define TILE_START		0x33	// 'S'
#define TILE_EXIT		0x25	// 'E'
#define TILE_WALL		0x40	// Upper-left corner of wall tile
#define TILE_EMPTY		0x44	// Empty tile
#define TILE_HOLE		0x00	// Blank "lose condition" tile
#define TILE_ITEM		0x45	// Upper-left corner of item tile
#define TILE_ENEMY		0x10	// '0'

// Nametable position and length of HUD labels
#define HUD_LABELS_ADR 	(NTADR_A(4,2))
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
	// End of file marker for metasprite array
	MS_EOF
};
const unsigned char enemyMetasprite[] =
{
	0,-1,0x4d,0,
	8,-1,0x4e,0,
	0, 7,0x4f,0,
	8, 7,0x50,0,
	MS_EOF
};

// Pre-initialized update list used during gameplay
#define UPDATE_LIST_SIZE (8*3 + 8 + 6 + 1)
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
	
	// Horizontal update sequences
	// 5-digit HUD display for total collected items (all levels), initialized to "0"
	MSB(NTADR_A(11,2))|NT_UPD_HORZ,LSB(NTADR_A(11,2)),5,0x10,0x10,0x10,0x10,0x10,
	// 3-digit HUD display for percent clear, initialized to "0"
	MSB(NTADR_A(23,2))|NT_UPD_HORZ,LSB(NTADR_A(23,2)),3,0x10,0x10,0x10,
	
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

// Enemy variables
static unsigned char enemy_tileX;
static unsigned char enemy_tileY;

// Exit position
static unsigned char exit_tileX;
static unsigned char exit_tileY;

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
	updateList[27] = 0x10 + totalItemsCollected5;
	updateList[28] = 0x10 + totalItemsCollected4;
	updateList[29] = 0x10 + totalItemsCollected3;
	updateList[30] = 0x10 + totalItemsCollected2;
	updateList[31] = 0x10 + totalItemsCollected1;
	// Percent collected in current level
	updateList[35] = 0x10 + percentCollected/100;
	updateList[36] = 0x10 + percentCollected/10%10;
	updateList[37] = 0x10 + percentCollected%10;
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
	switch (gameLevel)
	{
		case 0: 	vram_unrle(level_test);		break;
		case 1:		vram_unrle(level_test2);	break;
		default:	vram_unrle(level_test3);	break;
	}
	
	// Load HUD
	vram_adr(HUD_LABELS_ADR);
	vram_write((unsigned char*)hudLabels, HUD_LABELS_LEN);
	
	// Set level map palette
	pal_bg(pal_level_test);
	
	// Set sprite palette
	pal_spr(palGameSpr);
	
	// Read level nametable from VRAM
	i16 = MAP_START_ADR;
	ptr = 0;
	wait = 0;
	levelItemsCount = 0;
	levelItemsCollected = 0;
	percentCollected = 0;
	
	enemy_tileX = 255;
	enemy_tileY = 255;
	
	// If first level, reset totalItemsCollected
	if (gameLevel == 0)
	{
		totalItemsCollected1 = 0;
		totalItemsCollected2 = 0;
		totalItemsCollected3 = 0;
		totalItemsCollected4 = 0;
		totalItemsCollected5 = 0;
	}
	
	// Read MAP_HEIGHT rows starting from below the HUD
	for (i = HUD_HEIGHT; i < MAP_HEIGHT+2; ++i)
	{
		// Read map data one row at a time (32 bytes) into nameRow
		vram_adr(i16);
		vram_read(nameRow, 32);
		// Reset address to start of row (since it gets advanced by vram_read)
		vram_adr(i16);
		
		// Each map tile is 2 bytes wide, so read map data at increments of 2 up to the maximum map width (bytes = tiles*2 = tiles<<1)
		for (j = 0; j < MAP_WIDTH<<1; j += 2)
		{
			spr = nameRow[j];
			
			// Handle each tile according to their type
			switch (spr)
			{
			case TILE_START:
				// Player position x and y are 16-bit variables in the format
				//	MSB		8 bits - tile coordinate
				//			4 bits - position within tile, TILE_SIZE_BIT
				//			4 bits - fixed point resolution, FP_BITS
				// To set tile coordinate, shift TILE_SIZE_BIT+FP_BITS left
				// Since the loop variable j increments by 2, also divide by 2 (>>1) to get x
				player_x = j << TILE_PLUS_FP_BITS >> 1;
				player_y = i << TILE_PLUS_FP_BITS;
				player_prevTileX = j >> 1;
				player_prevTileY = i;
				player_dir = DIR_NONE;
				player_nextDir = DIR_NONE;
				player_moveCounter = 0;
				// Speed increases every level
				player_speed = (START_SPEED + SPEED_UP_PER_LEVEL*gameLevel) << FP_BITS;
				// Remove start marker from map
				spr = TILE_EMPTY;
				break;
			case TILE_EXIT:
				// Save exit position
				exit_tileX = j >> 1;
				exit_tileY = i;
				break;
			case TILE_ENEMY:
				enemy_tileX = j >> 1;
				enemy_tileY = i;
				// Remove enemy marker from map
				spr = TILE_EMPTY;
				break;
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
		oam_meta_spr(enemy_tileX << TILE_SIZE_BIT,
					 enemy_tileY << TILE_SIZE_BIT,
					 16,
					 enemyMetasprite);
		
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
				// Play player spawn sfx
				sfx_play(SFX_RESPAWN1, 1);
				// Play game music
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
				if (player_x == exit_tileX << TILE_PLUS_FP_BITS &&
					player_y == exit_tileY << TILE_PLUS_FP_BITS)
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
					sfx_play(SFX_ITEM, 1);
					// Update level collect count
					++levelItemsCollected;
					
					// Get percent of items collected in current level
					percentCollected = levelItemsCollected*100/levelItemsCount;
					
					// Hide enemy upon reaching clear percent requirement
					if (percentCollected >= CLEAR_PERC_REQT)
					{
						enemy_tileX = 255;	// Lower right screen corner
						enemy_tileY = 255;
					}
					
					// Update total collect count
					incrementTotalItemsCollected();
					
					// Get address of tile in nametable
					i16 = NAM_ADR(player_nextTileX, player_nextTileY);
					
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
				i16 = NAM_ADR(player_prevTileX, player_prevTileY);
				
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
				
				// If both "blanker" and "emptier" work on the same tile,
				//	"blanker" takes priority
				if (updateList[12] == updateList[0] && updateList[13] == updateList[1])
				{
					updateList[0] = updateList[3] = updateList[6] = updateList[9] = 0x28;
					updateList[1] = updateList[4] = updateList[7] = updateList[10] = 0x00;
				}
				
				// Keep player moving in same direction until hitting a wall or until another possible move direction is selected
				checkPlayerMove(player_nextDir);
			}
			
			// Check lose condition: player collides with enemy
			if (percentCollected < CLEAR_PERC_REQT && 
				!(((player_x>>TILE_SIZE_BIT)+4)>=((enemy_tileX<<TILE_SIZE_BIT)+12) ||
				  ((player_x>>TILE_SIZE_BIT)+12)<((enemy_tileX<<TILE_SIZE_BIT)+4) ||
				  ((player_y>>TILE_SIZE_BIT)+4)>=((enemy_tileY<<TILE_SIZE_BIT)+12) ||
				  ((player_y>>TILE_SIZE_BIT)+12)<((enemy_tileY<<TILE_SIZE_BIT)+4)))
			{
				gameClear = FALSE;
				levelDone = TRUE;
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
		if (gameLevel == LEVEL_END)
		{
			gameDone = TRUE;
		}
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
