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
static void restartMenuTimer(void);


static void mf_realTempSelect(void);
static void mf_realTempDo(void);
static void mf_realTempLeave(void);
static void mf_setTempSelect(void);
static void mf_setTempDo(void);
static void mf_setTempLeave(void);
static void mf_rollSelect(void);
static void mf_rollDo(void);
static void mf_rollLeave(void);
static void mf_leafSelect(void);
static void mf_leafSelectAct(void);
static void mf_leafExit(void);
static void mf_sndenSelect(void);
static void mf_sndenDo(void);
static void mf_sndenLeave(void);
static void mf_autopoffSelect(void);
static void mf_autopoffDo(void);
static void mf_autopoffLeave(void);
static void mf_actpoffSelect(void);
static void mf_actpoffDo(void);
static void mf_actpoffLeave(void);
static void mf_calibP1Select(void);
static void mf_calibP2Select(void);
static void mf_calibDo(void);
static void mf_calibDoExit(void);
static void mf_cdoneSelect(void);
static void mf_cdoneDo(void);

static void applyCalibrationPoint(uint8_t cpointNum, uint8_t cpointVal);

static uint8_t selectedMenuItemID;
static MenuFunctionRecord selectedMenuFunctionRecord;
static uint8_t jumpFlags;
static uint8_t setupValue_u8;

static uint8_t cpointNum;

static SoftTimer8b_t menuTimer = {		// used for menu state jumps
	.Timer = 0,	
	.Enabled = 0,
	.RunOnce = 1
};		

static SoftTimer8b_t userTimer = {		// used for display blinking
	.Enabled = 0,
	.RunOnce = 0	
};		




const PROGMEM MenuJumpRecord menuJumpSet[] = 
{
// sizeOf(MenuJumpRecord) = 5 bytes	
//	|	Item		|		Jump condition		|	Next item	|	Flags/Timeout		|
	// Real temperature indication
	{ mi_REALTEMP, 	BD_UP | BD_DOWN,				mi_SETTEMP,		SHIFT_LEFT	|	10	},
	{ mi_REALTEMP, 	BS_MENU,						mi_ROLL,		SHIFT_RIGHT	|	0	},
	{ mi_REALTEMP, 	BL_MENU,						mi_SNDEN,						10	},	
	// Rolling
	{ mi_ROLL, 		BS_MENU,						mi_REALTEMP,	SHIFT_LEFT	|	0	},
	{ mi_ROLL, 		BL_MENU,						mi_SNDEN,						10	},
	// Temperature control
	{ mi_SETTEMP, 	BS_MENU | TMR_EXP,				mi_REALTEMP,	SHIFT_RIGHT	| 	0	},
	{ mi_SETTEMP, 	BL_MENU,						mi_REALTEMP,	SHIFT_RIGHT	| DISCARD_CHANGES	| 	0		},
	// Sound enable/disable
	{ mi_SNDEN, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_SNDEN, 	BD_DOWN,						mi_AUTOPOFF,	SHIFT_RIGHT	|	10	},
	{ mi_SNDEN, 	BD_UP,							mi_CALIB2,		SHIFT_LEFT	|	10	},
	{ mi_SNDEN, 	BS_MENU,						mi_ACTSNDEN,					10	},
	{ mi_ACTSNDEN, 	BS_MENU |  TMR_EXP,				mi_SNDEN,						10	},
	{ mi_ACTSNDEN, 	BL_MENU,						mi_SNDEN,		DISCARD_CHANGES	|	10	},
	// Auto power off timeout
	{ mi_AUTOPOFF, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_AUTOPOFF, 	BD_DOWN,						mi_CALIB1,		SHIFT_RIGHT	|	10	},
	{ mi_AUTOPOFF, 	BD_UP,							mi_SNDEN,		SHIFT_LEFT	|	10	},
	{ mi_AUTOPOFF,		BS_MENU,					mi_ACTAUTOPOFF,					10	},
	{ mi_ACTAUTOPOFF,	BS_MENU | TMR_EXP,			mi_AUTOPOFF,				10	},
	{ mi_ACTAUTOPOFF,	BL_MENU,					mi_AUTOPOFF,	DISCARD_CHANGES	|	10	},
	// Calibration
	{ mi_CALIB1, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_CALIB1, 	BD_DOWN,						mi_CALIB2,		SHIFT_RIGHT	|	10	},
	{ mi_CALIB1, 	BD_UP,							mi_AUTOPOFF,	SHIFT_LEFT	|	10	},
	{ mi_CALIB1, 	BS_MENU,						mi_DOCALIB1,					0	},
	{ mi_CALIB2, 	BL_MENU | TMR_EXP,				mi_REALTEMP,					0	},
	{ mi_CALIB2, 	BD_DOWN,						mi_SNDEN,		SHIFT_RIGHT	|	10	},
	{ mi_CALIB2, 	BD_UP,							mi_CALIB1,		SHIFT_LEFT	|	10	},
	{ mi_CALIB2, 	BS_MENU,						mi_DOCALIB2,					0	},
	{ mi_DOCALIB1, 	BL_MENU,						mi_CALIB1,						10	},
	{ mi_DOCALIB1, 	BS_MENU,						mi_CDONE1,						5	},
	{ mi_CDONE1, 	BS_MENU | BL_MENU | TMR_EXP,	mi_REALTEMP,					0	},
	{ mi_DOCALIB2, 	BL_MENU,						mi_CALIB2,						10	},
	{ mi_DOCALIB2, 	BS_MENU,						mi_CDONE2,						5	},
	{ mi_CDONE2, 	BS_MENU | BL_MENU | TMR_EXP,	mi_REALTEMP,					0	},
	// Auto power off jumps - only from states without timeout, excluding calibration
	{ mi_REALTEMP, 	GOTO_POFF,						mi_POFFACT,						0	},	
	{ mi_ROLL, 		GOTO_POFF,						mi_POFFACT,						0	},	
	{ mi_POFFACT, 	BD_UP | BD_DOWN | BS_MENU | BD_ROTFWD | BD_ROTREV | BD_HEATCTRL, mi_REALTEMP,	0	}
 };
 

 const PROGMEM MenuFunctionRecord menuFunctionSet[]=
{
// sizeOf(MenuFunctionRecord) = 7 bytes
//	|	Item		|	Select func			|	Do func			|		Leave func		|
	{ mi_REALTEMP,		mf_realTempSelect, 		mf_realTempDo, 		mf_realTempLeave	},
	{ mi_SETTEMP,  		mf_setTempSelect, 		mf_setTempDo, 		mf_setTempLeave		},
	{ mi_ROLL, 			mf_rollSelect, 			mf_rollDo, 			mf_rollLeave		},
	{ mi_SNDEN,			mf_sndenSelect, 		mf_sndenDo,				0				},
	{ mi_ACTSNDEN,		mf_leafSelectAct, 		mf_sndenDo,			mf_sndenLeave		},
	{ mi_AUTOPOFF,		mf_autopoffSelect, 		mf_autopoffDo,			0				},
	{ mi_ACTAUTOPOFF,	mf_leafSelectAct,		mf_autopoffDo,		mf_autopoffLeave	},
	
	{ mi_CALIB1,		mf_calibP1Select,		mf_calibDo,				0				},
	{ mi_DOCALIB1,		mf_leafSelectAct, 		mf_calibDo,			mf_calibDoExit		},
	{ mi_CALIB2,		mf_calibP2Select,		mf_calibDo,				0				},
	{ mi_DOCALIB2,		mf_leafSelectAct, 		mf_calibDo,			mf_calibDoExit		},
	{ mi_CDONE1,		mf_cdoneSelect, 		mf_cdoneDo,				0				},
	{ mi_CDONE2,		mf_cdoneSelect, 		mf_cdoneDo,				0				},
	
	{ mi_POFFACT,		mf_actpoffSelect,		mf_actpoffDo,		mf_actpoffLeave		}
}; 

const PROGMEM char ms_realTempDo[] =	{' ',' ',' ',' ',0xB0,'C',0};
//const PROGMEM char ms_setTempDo[] =	{' ',' ',' ',' ',0xB0,'C',0};
const PROGMEM char ms_rollDo[] =		{' ',' ',' ',' ',' ',' ',0};
const PROGMEM char ms_soundEnDo[] =		{'S','N','D',' ',' ',' ',0};
const PROGMEM char ms_autoPoffDo[] =	{'O','F','F',' ',' ',' ',0};
const PROGMEM char ms_calibDo[] =		{' ',' ',' ',0};

char temp_str[10];


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
	if (sys_timers_flags & AUTOPOFF_EXPIRED)
		jumpCondition |= GOTO_POFF;
	
	// Get next menu item according to current state and jump conditions
	nextItem = getNextMenuItem(selectedMenuItemID, jumpCondition);
	
	// If next item differs from current
	if (nextItem.ItemID != selectedMenuItemID)
	{		
		jumpFlags = nextItem.Flags;
		// Call exit function for current item
		processItemFunction(selectedMenuFunctionRecord.LeaveFunction);
		
		// If shifting is specified, start it before calling any 
		//	new menu item functions 
		if (nextItem.Flags & SHIFT_RIGHT)
			startShiftingWindowRight();
		else if (nextItem.Flags & SHIFT_LEFT)
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
		memcpy_P(&jRecord,&menuJumpSet[i],sizeof(MenuJumpRecord));
		if (jRecord.Item == selectedItemId)				// If ID match,
		{
			if ((jRecord.JumpCondition & jmpCond) != 0)		// if any of jump conditions match too,
			{
				nextItem.ItemID = jRecord.NextItem;			// switch to next menu item
				nextItem.ItemTimeout = jRecord.Flags & TIMEOUT_MASK;
				nextItem.Flags = jRecord.Flags & ~TIMEOUT_MASK;
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
		memcpy_P(menuRecord,&menuFunctionSet[i],sizeof(MenuFunctionRecord));
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


//------------------------------------------------//
// Menu item "Real temperature indication"
// TOP level
//------------------------------------------------//
void mf_realTempSelect(void)
{
	setExtraLeds(LED_TEMP);
}

void mf_realTempDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_realTempDo,7);
	
	if (adc_status & (SENSOR_ERROR_NO_PRESENT))
	{
		printLedBuffer(0,"ERR 01");
	}
	else if (adc_status & (SENSOR_ERROR_SHORTED))
	{
		printLedBuffer(0,"ERR 02");
	}
	else
	{
		// Output ADC result to LED
		i32toa_align_right((int32_t)adc_celsius,str,NO_TERMINATING_ZERO | 4);
		printLedBuffer(0,str);
	}
}

void mf_realTempLeave(void)
{
	clearExtraLeds(LED_TEMP);
}


//------------------------------------------------//
// Menu item "Temperature setting"
// TOP level
//------------------------------------------------//
void mf_setTempSelect(void)
{
	clearExtraLeds(LED_TEMP);
	mf_leafSelectAct();						// setup and start timer
	setupValue_u8 = p.setup_temp_value;		// Make a copy of parameter being changed
}

void mf_setTempDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_realTempDo,7);
	
	if (button_state & (BD_UP | BR_UP))
	{
		if (setupValue_u8 < MAX_SET_TEMP)
			setupValue_u8 += TEMP_SET_STEP;
		restartMenuTimer();
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (setupValue_u8 > MIN_SET_TEMP)
			setupValue_u8 -= TEMP_SET_STEP;
		restartMenuTimer();
	}					
		
	// Output setting to LED
	if (setupValue_u8 < MAX_SET_TEMP)
	{
		u16toa_align_right(setupValue_u8,str,NO_TERMINATING_ZERO | 4);
		printLedBuffer(0,str);
	}		
	else
	{
		printLedBuffer(0," UNREG");
	}
	
	if (userTimer.FA_GE)
		setExtraLeds(LED_TEMP);
	else
		clearExtraLeds(LED_TEMP);
}

void mf_setTempLeave(void)
{
	mf_leafExit();
	if (!(jumpFlags & DISCARD_CHANGES))
	{
		p.setup_temp_value = setupValue_u8;		// Apply changes		
	}	
}

//------------------------------------------------//
// Menu item "Cyclic rolling"
// TOP level
//------------------------------------------------//

void mf_rollSelect(void)
{
	mf_leafSelectAct();		// setup and start timer
	setExtraLeds(LED_ROLL);
}

void mf_rollDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_rollDo,7);
		
	if (button_state & (BD_UP | BR_UP))
	{
		if (p.rollCycleSet < MAX_ROLL_CYCLES)
			p.rollCycleSet += ROLL_CYCLES_STEP;
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (p.rollCycleSet > MIN_ROLL_CYCLES)
			p.rollCycleSet -= ROLL_CYCLES_STEP;
	}	
		
	u16toa_align_right(p.rollCycleSet,str + 4,NO_TERMINATING_ZERO | 2);
	
	if ((!(rollState & ROLL_CYCLE)) || (userTimer.FA_GE))
	{
		u16toa_align_right(activeRollCycle,str + 1,NO_TERMINATING_ZERO | 2);
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
	mf_leafExit();
	clearExtraLeds(LED_ROLL);
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


//------------------------------------------------//
// Menu item "Sound enable/disable"
//------------------------------------------------//
void mf_sndenSelect(void)
{
	mf_leafSelect();					
	setupValue_u8 = p.sound_enable;		// Make a copy of parameter being changed
}

void mf_sndenDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_soundEnDo,7);	
		
	if (button_state & (BD_UP | BD_DOWN))
	{
		setupValue_u8 = !setupValue_u8;
		restartMenuTimer();
	}			
		
	if (userTimer.FA_GE)
	{
		if (setupValue_u8)		
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

void mf_sndenLeave(void)
{
	mf_leafExit();
	if (!(jumpFlags & DISCARD_CHANGES))
	{
		p.sound_enable = setupValue_u8;		// Apply changes
	}
}


//------------------------------------------------//
// Menu item "Auto power off setup"
//------------------------------------------------//
void mf_autopoffSelect(void)
{
	mf_leafSelect();						
	setupValue_u8 = p.power_off_timeout;	// Make a copy of parameter being changed
}

void mf_autopoffDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_autoPoffDo,7);	
		
	if (button_state & (BD_UP | BR_UP))
	{
		if (setupValue_u8 < MAX_POWEROFF_TIMEOUT)
			setupValue_u8 += POWEROFF_SET_STEP;
		restartMenuTimer();
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (setupValue_u8 > MIN_POWEROFF_TIMEOUT)
			setupValue_u8 -= POWEROFF_SET_STEP;
		restartMenuTimer();
	}	
		
	if (userTimer.FA_GE)
	{
		if (setupValue_u8 < MAX_POWEROFF_TIMEOUT)
		{
			u16toa_align_right(setupValue_u8,str + 4,NO_TERMINATING_ZERO | 2);	
		}			
		else 
		{
			str[4] = 'N';
			str[5] = 'O';
		}			
	}		
		
	printLedBuffer(0,str);
}

void mf_autopoffLeave(void)
{
	mf_leafExit();
	if (!(jumpFlags & DISCARD_CHANGES))
	{
		p.power_off_timeout = setupValue_u8;		// Apply changes
	}
}


//------------------------------------------------//
// Menu item "Active power off"
// Specific menu item - used as indicator of system state
//------------------------------------------------//
void mf_actpoffSelect(void)
{
	clearExtraLeds(LED_TEMP | LED_ROLL);
	autoPowerOffState = AUTO_POFF_ACTIVE;	// Set global flag
}

// Indication of power off mode
void mf_actpoffDo(void)
{
	printLedBuffer(0,"   OFF");
}

void mf_actpoffLeave(void)
{
	autoPowerOffState = 0;	
}




//------------------------------------------------//
// Menu item "Calibration"
// There are two calibration points - the menu item is 
// same for both.
//------------------------------------------------//
void mf_calibP1Select(void)
{
	mf_leafSelect();
	setupValue_u8 = cp.cpoint1;	// determine which point to use at select func
	cpointNum = 1;
	printLedBuffer(0,"P1    ");
}

void mf_calibP2Select(void)
{
	mf_leafSelect();
	setupValue_u8 = cp.cpoint2;	// determine which point to use at select func
	cpointNum = 2;
	printLedBuffer(0,"P2    ");
}

void mf_calibDo(void)
{
	char *str = (char *)&temp_str;
	memcpy_P(str,&ms_calibDo,4);
	
	if (button_state & (BD_UP | BR_UP))
	{
		if (setupValue_u8 < MAX_CALIB_TEMP)
		setupValue_u8 += CALIB_TEMP_STEP;
	}
	else if (button_state & (BD_DOWN | BR_DOWN))
	{
		if (setupValue_u8 > MIN_CALIB_TEMP)
		setupValue_u8 -= CALIB_TEMP_STEP;
	}
	
	if (userTimer.FA_GE)
	{
		u16toa_align_right(setupValue_u8,str,3);
		resetAutoPowerOffCounter();
		heaterState |= CALIBRATION_ACTIVE;
	}
	
	printLedBuffer(3,str);
}


void mf_calibDoExit(void)
{
	mf_leafExit();
	heaterState &= ~CALIBRATION_ACTIVE;
}



//------------------------------------------------//
// Menu item "Calibration done"
// There are two calibration points - the menu item is
// same for both.
//------------------------------------------------//
void mf_cdoneSelect(void)
{
	applyCalibrationPoint(cpointNum,setupValue_u8);
}

void applyCalibrationPoint(uint8_t cpointNum, uint8_t cpointVal)
{
	update_CalibrationPoint(cpointNum,cpointVal);
	calculateCoeffs();
	saveCalibrationToEEPROM();
}

void mf_cdoneDo(void)
{
	printLedBuffer(0," DONE ");
}






