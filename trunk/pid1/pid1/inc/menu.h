/*
 * menu_func.h
 *
 * Created: 12.04.2013 16:52:18
 *  Author: Avega
 */ 


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
	uint8_t ShiftFlags;
} NextItem_t;


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
#define 	TIMEOUT_MASK	0x3F	// timeouts = 0 to 63 (in units of processMenu() call period)

// Jump condition flags:
#define		TMR_EXP			0x8000
#define 	GOTO_POFF		0x4000


// Time intervals
#define MENU_TIMEOUT_MULT	2		// multiplier of menu item timeouts (1 to 4)
#define BLINK_PERIOD		8		// in units of processMenu() call period



void InitMenu(void);
void processMenu(void);



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

static void mf_sndenDo(void);
static void mf_autopoffDo(void);
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



