


///////////////////////////////////////

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

#define RINIT	0x01
#define RNORM	0x00

typedef struct {
	const uint8_t length;
	uint16_t *data;
	uint32_t summ;
	uint8_t curr_pos;
	uint8_t stat;
} RingBufU16_t;



void ringTestFunc1(void);

///////////////////////////////////////



///////////////////////////////////////

#define ADC_BUFFER_LENGTH	8
#define PID_ERR_BUF_LENGTH	10

uint16_t raw_adc_buffer[ADC_BUFFER_LENGTH];
RingBufU16_t ringBufADC = {
	.length = ADC_BUFFER_LENGTH,
	.data = raw_adc_buffer,
	.stat = RINIT
};


uint16_t pid_err_buffer[PID_ERR_BUF_LENGTH];
RingBufU16_t ringBufErr = {
	.length = PID_ERR_BUF_LENGTH,
	.data = pid_err_buffer,
	.stat = RINIT
};



void addToRingU16(RingBufU16_t* bptr, uint16_t sample)
{
	if (bptr->stat == RNORM)
	{
		bptr->summ -= bptr->data[bptr->curr_pos];
	}
	else
	{
		bptr->curr_pos = 0;
		bptr->summ = 0;
	}
	do	
	{
		bptr->data[bptr->curr_pos++] = sample;
		bptr->summ += sample;
		if (bptr->curr_pos == bptr->length)	
		{	
			bptr->curr_pos = 0;	
			bptr->stat = RNORM;
		}
	} 
	while (bptr->stat != RNORM);
}



void ringTestFunc1(void)
{
	uint16_t s = 1000;
	while(1)
	{
		addToRingU16(&ringBufADC, s);
		s++;
		
		
	}
	
	
}

///////////////////////////////////////






