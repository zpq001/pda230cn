


typedef struct {
	const uint8_t length;
	uint16_t data[];
	uint32_t summ;
	uint8_t curr_pos;
} RingBufU16_t;


void addToRingU16(RingBufU16_t* bptr, uint16_t sample)
{
	bptr->summ -= bptr->data[bptr->curr_pos];
	bptr->data[bptr->curr_pos] = sample;
	bptr->summ += sample;
	
	bptr->curr_pos++;
	if (bptr->curr_pos == bptr->length)
		bptr->curr_pos = 0;	
}








