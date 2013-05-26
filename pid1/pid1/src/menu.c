/*
 * menu.c
 *
 * Created: 12.04.2013 16:51:32
 *  Author: Avega
 */ 


#include "compilers.h"
#include "menu.h"
#include "leds.h"
#include "buttons.h"
#include "adc.h"
#include "systimer.h"
#include "control.h"
#include "my_string.h"
#include "soft_timer.h"
#include "led_indic.h"
#include "power_control.h"


static inline NextItem_t getNextMenuItem(uint8_t selectedItemId, uint16_t jmpCond);
static void getMenuFunctionRecord(uint8_t menuItemID, MenuFunctionRecord* menuRecord );
static inline void processItemFunction(FuncPtr funcAddr);
static inline void readMenuRecord(const MenuFunctionRecord* fRecPtr, MenuFunctionRecord* resPtr );
static inline void readJumpRecord(const MenuJumpRecord* jRecPtr, MenuJumpRecord* resPtr );
static void restartMenuTimer(void);

static uint8_t selectedMenuItemID;
static MenuFunctionRecord selectedMenuFunctionRecord;

static uint8_t cpoint1_copy;
static uint8_t cpoint2_copy;

static SoftTimer8b_t menuTimer = {		// used for menu state jumps
	.Timer = 0,	
	.Enabled = 0,
	.RunOnce = 1
};		

static SoftTimer8b_t userTimer = {		// used for display blinking
	.Enabled = 0,
	.RunOnce = 0	
};		




const __flash MenuJumpRecord menuJumpSet[] = 
{
// sizeOf(MenuJumpRecord) = 5 bytes	
//	|	Item		|		Jump condition		|	Next item	|	Flags/Timeout		|
	// Real temperature indication
	{ mi_REALTEMP, 	BD_UP | BD_DOWN,				mi_SETTEMP,		SHIFT_LEFT	|	40	},
	{ mi_REALTEMP, 	BS_MENU,						mi_ROLL,		SHIFT_RIGHT	|	0	},
	{ mi_REALTEMP, 	BL_MENU,						mi_SNDEN,						40	},	
	// Rolling
	{ mi_ROLL, 		BS_MENU,						mi_REALTEMP,	SHIFT_LEFT	|	0	},
	// Temperature control
	{ mi_SETTEMP, 	BS_MENU | BL_MENU | TMR_EXP,	mi_REALTEMP,	SHIFT_RIGHT	| 	0	},
	// Sound enable/disable
	{ mi_SNDEN, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_SNDEN, 	BD_DOWN,						mi_AUTOPOFF,	SHIFT_RIGHT	|	40	},
	{ mi_SNDEN, 	BD_UP,							mi_CALIB2,		SHIFT_LEFT	|	40	},
	{ mi_SNDEN, 	BS_MENU,						mi_ACTSNDEN,					40	},
	{ mi_ACTSNDEN, 	BS_MENU | BL_MENU |  TMR_EXP,	mi_SNDEN,						40	},
	// Auto power off timeout
	{ mi_AUTOPOFF, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_AUTOPOFF, 	BD_DOWN,						mi_CALIB1,		SHIFT_RIGHT	|	40	},
	{ mi_AUTOPOFF, 	BD_UP,							mi_SNDEN,		SHIFT_LEFT	|	40	},
	{ mi_AUTOPOFF,		BS_MENU,					mi_ACTAUTOPOFF,					40	},
	{ mi_ACTAUTOPOFF,	BS_MENU | BL_MENU | TMR_EXP,	mi_AUTOPOFF,				40	},
	// Calibration
	{ mi_CALIB1, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_CALIB1, 	BD_DOWN,						mi_CALIB2,		SHIFT_RIGHT	|	40	},
	{ mi_CALIB1, 	BD_UP,							mi_AUTOPOFF,	SHIFT_LEFT	|	40	},
	{ mi_CALIB1, 	BS_MENU,						mi_DOCALIB1,					0	},
	{ mi_CALIB2, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_CALIB2, 	BD_DOWN,						mi_SNDEN,		SHIFT_RIGHT	|	40	},
	{ mi_CALIB2, 	BD_UP,							mi_CALIB1,		SHIFT_LEFT	|	40	},
	{ mi_CALIB2, 	BS_MENU,						mi_DOCALIB2,					0	},
	{ mi_DOCALIB1, 	BL_MENU,						mi_CALIB1,						0	},
	{ mi_DOCALIB1, 	BS_MENU,						mi_CDONE1,						20	},
	{ mi_CDONE1, 	BS_MENU | BL_MENU | TMR_EXP,	mi_REALTEMP,					0	},
	{ mi_DOCALIB2, 	BL_MENU,						mi_CALIB2,						0	},
	{ mi_DOCALIB2, 	BS_MENU,						mi_CDONE2,						20	},
	{ mi_CDONE2, 	BS_MENU | BL_MENU | TMR_EXP,	mi_REALTEMP,					0	}		
 };
 

 const __flash MenuFunctionRecord menuFunctionSet[]=
{
// sizeOf(MenuFunctionRecord) = 7 bytes
// total 91 byte
//	|	Item		|	Select func			|	Do func			|		Leave func		|
	{ mi_REALTEMP,		mf_realTempSelect, 		mf_realTempDo, 		mf_realTempLeave	},
	{ mi_SETTEMP,  		mf_setTempSelect, 		mf_setTempDo, 		mf_setTempLeave		},
	{ mi_ROLL, 			mf_rollSelect, 			mf_rollDo, 			mf_rollLeave		},
	{ mi_SNDEN,			mf_leafSelect, 			mf_sndenDo,				0				},
	{ mi_ACTSNDEN,		mf_leafSelectAct, 		mf_sndenDo,			mf_leafExit			},
	{ mi_AUTOPOFF,		mf_leafSelect, 			mf_autopoffDo,			0				},
	{ mi_ACTAUTOPOFF,	mf_leafSelectAct,		mf_autopoffDo,		mf_leafExit			},
	
	{ mi_CALIB1,		mf_calibSelect,			mf_calib1Do,			0				},
	{ mi_DOCALIB1,		mf_leafSelectAct, 		mf_calib1Do,		mf_leafExit			},
	{ mi_CALIB2,		mf_calibSelect,			mf_calib2Do,			0				},
	{ mi_DOCALIB2,		mf_leafSelectAct, 		mf_calib2Do,		mf_leafExit			},
	{ mi_CDONE1,		mf_cdone1Select, 		mf_cdoneDo,				0				},
	{ mi_CDONE2,		mf_cdone2Select, 		mf_cdoneDo,				0				}
}; 




//=================================================================//
//=================================================================//
//=================================================================//



//-----------------------------------------------------------------//
//	Menu initialization
//	Call this function at start of your program before processMenu()
//-----------------------------------------------------------------//
void InitMenu(void)
{
	selectedMenuItemID = mi_REALTEMP;
	getMenuFunctionRecord(selectedMenuItemID, &selectedMenuFunctionRecord);
	processItemFunction(selectedMenuFunctionRecord.SelectFunction);
}



//-----------------------------------------------------------------//
//	Main menu control, -  state function calls, jumps 
//	between menu states and indication
//	Call this function from main() with a fixed time interval.
//-----------------------------------------------------------------//
void processMenu(void)
{
	NextItem_t nextItem;
	uint16_t jumpCondition;
	
	processSoftTimer8b(&menuTimer);	
	
	// Compose jump condition
	jumpCondition = button_state;		
	if (menuTimer.FTop)
		jumpCondition |= TMR_EXP;
	
	// Get next menu item according to current state and jump conditions
	nextItem = getNextMenuItem(selectedMenuItemID, jumpCondition);
	
	// If next item differs from current
	if (nextItem.ItemID != selectedMenuItemID)
	{		
		// Call exit function for current item
		processItemFunction(selectedMenuFunctionRecord.LeaveFunction);
		
		// If shifting is specified, start it before calling any 
		//	new menu item functions 
		if (nextItem.ShiftRight)
			startShiftingWindowRight();
		else if (nextItem.ShiftLeft)
			startShiftingWindowLeft();
		
		// Select new item
		selectedMenuItemID = nextItem.ItemID;
		
		// Load from FLASH and save function pointers
		getMenuFunctionRecord(selectedMenuItemID, &selectedMenuFunctionRecord);
		// Call select function for next item
		processItemFunction(selectedMenuFunctionRecord.SelectFunction);
		// Start timer if timeout is enabled
		if (nextItem.ItemTimeout != 0)
		{
			menuTimer.Top = nextItem.ItemTimeout * MENU_TIMEOUT_MULT;
			menuTimer.Timer = 0;
			menuTimer.Enabled = 1;	
		}
		
	}
	else
	{
		processSoftTimer8b(&userTimer);
		// Stay at previous item
		processItemFunction(selectedMenuFunctionRecord.RunFunction);
	}	
}





//-----------------------------------------------------------------//
//	Returns ID of next menu item. 
// If current item ID and jump condition match any of the
// records in the "menuJumpSet" table new ID is returned.
// If there is no record match, selectedItemId is returned.
//	Arguments:
//		selectedItemId	- ID of currently active item
//		jmpCond			- all information for jumps between states (buttons, timer flags, etc.)
//	Output:
//		NextItemID		- ID of item to jump to.
//		NextItemTimeout	- allowed timeout for next item. Do not care if next item is the current.
//-----------------------------------------------------------------//
static inline NextItem_t getNextMenuItem(uint8_t selectedItemId, uint16_t jmpCond)
{
	NextItem_t nextItem;
	nextItem.ItemID = selectedItemId;
	MenuJumpRecord	jRecord;
	uint8_t i;
	
	for (i = 0; i < sizeof(menuJumpSet)/sizeof(MenuJumpRecord); i++  )
	{
		readJumpRecord(&menuJumpSet[i], &jRecord);		// read full jump record
		if (jRecord.Item == selectedItemId)				// If ID match,
		{
			if ((jRecord.JumpCondition & jmpCond) != 0)		// if any of jump conditions match too,
			{
				nextItem.ItemID = jRecord.NextItem;			// switch to next menu item
				nextItem.ItemTimeout = jRecord.Flags & TIMEOUT_MASK;
				nextItem.ShiftRight = (jRecord.Flags & SHIFT_RIGHT) ? 1 : 0;
				nextItem.ShiftLeft = (jRecord.Flags & SHIFT_LEFT) ? 1 : 0;
				break;
			}
		}
	}
	return nextItem;
}


//-----------------------------------------------------------------//
//	Finds function pointers record for menuItemID
//	If menuItemID is not found, last record in the 
//	menuFunctionSet table is returned.
//	Arguments:
//		menuItemID - ID of an item
//		menuRecord - pointer to function structure to fill
//	Output:
//-----------------------------------------------------------------//
static void getMenuFunctionRecord(uint8_t menuItemID, MenuFunctionRecord* menuRecord )
{
	uint8_t i;
	for (i = 0; i < sizeof(menuFunctionSet)/sizeof(MenuFunctionRecord); i++  )
	{
		readMenuRecord(&menuFunctionSet[i], menuRecord);
		if (menuRecord->Item == menuItemID)
			break;
	}
}


//-----------------------------------------------------------------//
//	Runs function at specified address
//	Arguments:
//		funcAddr - address of the function to run.
//-----------------------------------------------------------------//
static inline void processItemFunction(FuncPtr funcAddr)
{
	if (funcAddr != 0)
		((FuncPtr)funcAddr)();
}



//-----------------------------------------------------------------//
//	 Reads menu function record from FLASH into SRAM memory
//	Arguments:
//		fRecPtr		- address of the function record in the FLASH
//		resPtr		- address of the record to fill in the SRAM
//-----------------------------------------------------------------//
static inline void readMenuRecord(const MenuFunctionRecord* fRecPtr, MenuFunctionRecord* resPtr )
{
	resPtr->Item = pgm_read_byte(&fRecPtr->Item);
	resPtr->SelectFunction = (FuncPtr)pgm_read_word(&fRecPtr->SelectFunction);
	resPtr->RunFunction = (FuncPtr)pgm_read_word(&fRecPtr->RunFunction);
	resPtr->LeaveFunction = (FuncPtr)pgm_read_word(&fRecPtr->LeaveFunction);
}


//-----------------------------------------------------------------//
//	 Reads jump record from FLASH into SRAM memory
//	Arguments:
//		jRecPtr		- address of the jump record in the FLASH
//		resPtr		- address of the record to fill in the SRAM
//-----------------------------------------------------------------//
static inline void readJumpRecord(const MenuJumpRecord* jRecPtr, MenuJumpRecord* resPtr )
{
	resPtr->Item = pgm_read_byte(&jRecPtr->Item);
	resPtr->JumpCondition = pgm_read_word(&jRecPtr->JumpCondition);
	resPtr->NextItem= pgm_read_byte(&jRecPtr->NextItem);
	resPtr->Flags= pgm_read_byte(&jRecPtr->Flags);
}


//-----------------------------------------------------------------//
//	 Restarts menu state timeout
//	Should be used in RUN function when control buttons are pressed.
//-----------------------------------------------------------------//
static void restartMenuTimer(void)
{
	menuTimer.Timer = 0;
}





//=================================================================//
//=================================================================//
//=================================================================//


void mf_realTempSelect(void)
{
	setExtraLeds(LED_TEMP);
}

void mf_realTempDo(void)
{
	char str[] = {' ',' ',' ',' ',0xB0,'C',0};
	// Output ADC result to LED
	u16toa_align_right(adc_celsius,str,0x80 | 4,' ');
	printLedBuffer(0,str);
}

void mf_realTempLeave(void)
{
	clearExtraLeds(LED_TEMP);
}

//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//


void mf_setTempSelect(void)
{
	clearExtraLeds(LED_TEMP);
	mf_leafSelectAct();		// setup and start timer
}

void mf_setTempDo(void)
{
	char str[] = {' ',' ',' ',' ',0xB0,'C',0};

	
	if (button_state & (BD_UP | BR_UP))
	{
		if (setup_temp_value < MAX_SET_TEMP)
			setup_temp_value += 5;
		restartMenuTimer();
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (setup_temp_value > MIN_SET_TEMP)
			setup_temp_value -= 5;
		restartMenuTimer();
	}					
		
	// Output ADC result to LED
	u16toa_align_right(setup_temp_value,str,0x80 | 4,' ');
	printLedBuffer(0,str);
	
	if (userTimer.FA_GE)
		setExtraLeds(LED_TEMP);
	else
		clearExtraLeds(LED_TEMP);
}

void mf_setTempLeave(void)
{
	userTimer.Enabled = 0;
}

//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//

void mf_rollSelect(void)
{
	setExtraLeds(LED_ROLL);
	mf_leafSelectAct();		// setup and start timer
}

void mf_rollDo(void)
{
	char str[] = {' ',' ',' ',' ',' ',' ',0};
		
	if (button_state & (BD_UP | BR_UP))
	{
		if (rollCycleSet < MAX_ROLL_CYCLES)
		rollCycleSet += 1;
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (rollCycleSet > MIN_ROLL_CYCLES)
		rollCycleSet -= 1;
	}	
		
	u16toa_align_right(rollCycleSet,str + 4,0x80 | 2,' ');
	
	if ((!(rollState & ROLL_CYCLE)) || (userTimer.FA_GE))
	{
		u16toa_align_right(activeRollCycle,str + 1,0x80 | 2,' ');
	}
	
	str[0] = 0;
	if (isTopPointValid())
		str[0] |= SEGA;
	if (isBottomPointValid())
		str[0] |= SEGD;
	if (str[0] == 0)
		str[0] = ' ';
		
	printLedBuffer(0,str);
}

void mf_rollLeave(void)
{
	clearExtraLeds(LED_ROLL);
	userTimer.Enabled = 0;
}

//---------------------------------------------//
//---------------------------------------------//
//---------------------------------------------//


// Select and exit functions, common for leaf menu items 

void mf_leafSelect(void)
{
	clearExtraLeds(LED_TEMP | LED_ROLL);
	userTimer.FA_GE = 1;	
}

void mf_leafSelectAct(void)
{
	userTimer.Timer = 0;
	userTimer.Top = BLINK_PERIOD - 1;
	userTimer.CompA = BLINK_PERIOD / 2;
	userTimer.Enabled = 1;
}

void mf_leafExit(void)
{
	userTimer.Enabled = 0;	
}

//---------------------------------------------//


void mf_sndenDo(void)
{
	char str[] = {'S','N','D',' ',' ',' ',0};
		
	if (button_state & (BD_UP | BD_DOWN))
	{
		sound_enable = !sound_enable;
		restartMenuTimer();
	}			
		
	if (userTimer.FA_GE)
	{
		if (sound_enable)		
		{
			str[4] = 'O';
			str[5] = 'N';
		}
		else
		{
			str[3] = 'O';
			str[4] = 'F';
			str[5] = 'F';
		}			
	}
		
	printLedBuffer(0,str);
	setComma(2);
}


//---------------------------------------------//


void mf_autopoffDo(void)
{
	char str[] = {'O','F','F',' ',' ',' ',0};
		
		
	if (button_state & (BD_UP | BR_UP))
	{
		if (power_off_timeout < MAX_POWEROFF_TIMEOUT)
			power_off_timeout += 5;
		restartMenuTimer();
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (power_off_timeout > MIN_POWEROFF_TIMEOUT)
			power_off_timeout -= 5;
		restartMenuTimer();
	}	
		
	if (userTimer.FA_GE)
	{
		if (power_off_timeout != MAX_POWEROFF_TIMEOUT)
			u16toa_align_right(power_off_timeout,str + 4,0x80 | 2,' ');	
		else 
		{
			str[4] = 'N';
			str[5] = 'O';
		}			
	}		
		
	printLedBuffer(0,str);
}

//---------------------------------------------//

void mf_calibSelect(void)
{
	mf_leafSelect();
	cpoint1_copy = cpoint1;
	cpoint2_copy = cpoint2;
}

void mf_calib1Do(void)
{
	char str[] = {'P','1',' ',' ',' ',' ',0};
	
	
	if (button_state & (BD_UP | BR_UP))
	{
		if (cpoint1_copy < MAX_CALIB_TEMP)
			cpoint1_copy += 1;
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (cpoint1_copy > MIN_CALIB_TEMP)
			cpoint1_copy -= 1;
	}
	
	if (userTimer.FA_GE)
	{
		u16toa_align_right(cpoint1_copy,str + 3,0x80 | 3,' ');
	}
	
	printLedBuffer(0,str);
}

//---------------------------------------------//

void mf_calib2Do(void)
{
	char str[] = {'P','2',' ',' ',' ',' ',0};
	
	
	if (button_state & (BD_UP | BR_UP))
	{
		if (cpoint2_copy < MAX_CALIB_TEMP)
		cpoint2_copy += 1;
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (cpoint2_copy > MIN_CALIB_TEMP)
		cpoint2_copy -= 1;
	}
	
	if (userTimer.FA_GE)
	{
		u16toa_align_right(cpoint2_copy,str + 3,0x80 | 3,' ');
	}
	
	printLedBuffer(0,str);
	
}

//---------------------------------------------//

void mf_cdone1Select(void)
{
	// Save current ADC as calibrating point
	cpoint1_adc = adc_normalized;
	// Save current Celsius degree
	cpoint1 = cpoint1_copy;
	// Calculate new coefficient for temperature conversion
	calculateCoeffs();
}

void mf_cdone2Select(void)
{
	// Save current ADC as calibrating point
	cpoint2_adc = adc_normalized;
	// Save current Celsius degree
	cpoint2 = cpoint2_copy;
	// Calculate new coefficient for temperature conversion
	calculateCoeffs();
}

void mf_cdoneDo(void)
{
	printLedBuffer(0," DONE ");
}






