

#define Kp		15
#define Ki		5
#define Kd		40
#define SCALING_FACTOR	5


typedef struct {
	uint8_t n;
	int16_t dc_gain;
	int8_t *coeffs;
} filter8bit_core_t;



typedef struct {
	const uint8_t length;
	uint16_t *data;
	uint32_t summ;
	uint8_t curr_pos;
	uint8_t stat;
} RingBufU16_t;


// ----- Ring buffer ----- //
#define RINIT	0x01
#define RNORM	0x00


extern int16_t dbg_PID_p_term;
extern int16_t dbg_PID_d_term;
extern int16_t dbg_PID_i_term;
extern int16_t dbg_PID_output;


	void initRegulator(void);
	uint8_t processPID(uint16_t setPoint, uint16_t processValue);


