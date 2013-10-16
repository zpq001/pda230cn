/*
 * menu_func.h
 *
 * Created: 12.04.2013 16:52:18
 *  Author: Avega
 */ 

//--------------------------------------------//
// Typedefs

typedef void (*FuncPtr)(void);

typedef struct {
	uint8_t		Item;				//
	uint16_t	JumpCondition;      //
	uint8_t		NextItem;			//
	uint8_t		Flags;
} MenuJumpRecord;


typedef struct {
	uint8_t				Item;				//
	FuncPtr				SelectFunction;     //
	FuncPtr				RunFunction;		//
	FuncPtr				LeaveFunction;      //
} MenuFunctionRecord;


typedef struct {
	uint8_t ItemID;
	uint8_t ItemTimeout;
	uint8_t Flags;
} NextItem_t;



//--------------------------------------------//
// Menu state and control defines

// Menu items
#define		mi_REALTEMP		0x01
#define		mi_ROLL			0x02
#define		mi_SETTEMP		0x03
#define		mi_AUTOPOFF		0x04
#define		mi_SNDEN		0x05
#define		mi_CALIB1		0x06
#define		mi_CALIB2		0x07
#define		mi_CDONE1		0x08
#define		mi_CDONE2		0x0D
#define		mi_DOCALIB1		0x09
#define		mi_DOCALIB2		0x0A
#define		mi_ACTSNDEN		0x0B
#define		mi_ACTAUTOPOFF	0x0C
#define		mi_POFFACT		0x0E

// MenuJumpRecord flags:
#define 	SHIFT_LEFT		0x80
#define 	SHIFT_RIGHT		0x40
#define		DISCARD_CHANGES	0x20
#define 	TIMEOUT_MASK	0x0F	// timeouts = 0 to 15 (in units of 4x processMenu() call period)

// Jump condition flags:
#define		TMR_EXP			0x8000	// Menu item timeout expired flag
#define 	GOTO_POFF		0x4000	// Auto power off counter expired flag
#define		BS_MENU			0x0100	// Short menu button press
#define		BL_MENU			0x0200	// Long menu button press

// Time intervals
#define MENU_TIMEOUT_MULT	8		// multiplier of menu item timeouts (1 to 16)
#define BLINK_PERIOD		8		// in units of processMenu() call period


//--------------------------------------------//
// Externs

void InitMenu(void);
void processMenu(void);






