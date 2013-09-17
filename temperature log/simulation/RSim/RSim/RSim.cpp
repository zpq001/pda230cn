// RSim.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <windows.h>
#include <iostream>

#include "stdafx.h"
#include "stdint.h"
#include "simulation.h"
#include "plant.h"
extern "C" {
	#include "pid_controller.h"
}

int _tmain(int argc, _TCHAR* argv[])
{
	int i;
	int pid_call_timer = 0;
	int regulator_counter = 0;
	
	float plantState;
	float processF;
	uint16_t processValue;
	float setPointF;
	uint16_t setPoint; 
	float effect = 0;
	
	float k_norm = 0.446;
	float offset_norm = 48.144;

	float tempSetting = 120;		// Desired temperature
	
	
	initPlant(25.0, 25.0); 

	// Calculate process value
	plantState = getPlantState();					
	processF = (plantState + offset_norm) / k_norm;	
	processF *= 4;
	//processF /= 2;
	processValue = (uint16_t)processF;
	initPID(processValue);	


	// Log data
	float log_state[LOG_SIZE];					// Plant state, float
	uint16_t log_state_int[LOG_SIZE];			// Plant state, truncated integer
	int16_t log_p_term[LOG_SIZE];
	int16_t log_d_term[LOG_SIZE];
	int16_t log_i_term[LOG_SIZE];
	uint8_t log_pid_output[LOG_SIZE];
	int log_index = 0;
	 
	//-------------------//


	for (i=0;i<(SIM_TIME / TIMESTEP);i++)
	{
		// Process plant with TIMESTEP interval
		processPlant(effect);	
		
		// Process regulator 
		if (pid_call_timer == (PID_CALL_INTERVAL / TIMESTEP - 1))
		{	
			// Calculate process value
			plantState = getPlantState();					
			processF = (plantState + offset_norm) / k_norm;	
			processF *= 4;
			//processF /= 2;
			processValue = (uint16_t)processF;
			
			// Calculate setpoint
			setPointF = (tempSetting + offset_norm) / k_norm;
			setPointF *= 4;
			//setPointF /= 2;
			setPoint = (uint16_t)setPointF;	
			
			// PID
			effect = processPID(setPoint, processValue);
			
			
			pid_call_timer = 0;
			regulator_counter++;
		}
		else
		{
			pid_call_timer++;
		}

		// LOG
		log_state[log_index] = getPlantState();
		log_state_int[log_index] = (uint16_t)log_state[log_index];
		log_p_term[log_index] = dbg_PID_p_term;
		log_i_term[log_index] = dbg_PID_i_term;
		log_d_term[log_index] = dbg_PID_d_term;
		log_pid_output[log_index] = dbg_PID_output;

		log_index++;
	}



	//-------------------------------//
	// Save results to files

	LPCWSTR outDir = L"output\\";
	char* fname_state_int =	    "output\\col_0.txt";
	char* fname_state_float =	"output\\col_0f.txt";
	char* fname_p_term =		"output\\col_5.txt";
	char* fname_d_term =		"output\\col_6.txt";
	char* fname_i_term =		"output\\col_7.txt";
	char* fname_pid_output =	"output\\col_8.txt";

	if (!CreateDirectory(outDir,NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			std::cout << "Cannot create directory" << std::endl;
			std::cin.get();
			return 0;
		}
	}
	
	FILE *f_state_float = fopen( fname_state_float, "w" ); 
	FILE *f_state_int =	fopen( fname_state_int, "w" ); 
	FILE *f_p_term = fopen( fname_p_term, "w" ); 
	FILE *f_d_term = fopen( fname_d_term, "w" ); 
	FILE *f_i_term = fopen( fname_i_term, "w" ); 
	FILE *f_pid_output = fopen( fname_pid_output, "w" ); 
	

	for(int i = 0; i < LOG_SIZE-1; i++)
	{
		fprintf(f_state_float, "%f\r", log_state[i]);
		fprintf(f_state_int, "%u\r", log_state_int[i]);
		fprintf(f_p_term, "%d\r", log_p_term[i]);
		fprintf(f_d_term, "%d\r", log_d_term[i]);
		fprintf(f_i_term, "%d\r", log_i_term[i]);
		fprintf(f_pid_output, "%d\r", log_pid_output[i]);
	}


	
	fclose(f_state_float);
	fclose(f_state_int);
	fclose(f_p_term);
	fclose(f_d_term);
	fclose(f_i_term);
	fclose(f_pid_output);
	

	return 0;
}

