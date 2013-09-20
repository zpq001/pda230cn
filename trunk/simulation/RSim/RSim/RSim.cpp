// RSim.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <windows.h>
#include <iostream>

#include <string>

#include "vector_reader.h"

#include "stdafx.h"
#include "stdint.h"
#include "simulation.h"
#include "plant.h"
extern "C" {
	#include "pid_controller.h"
}

int _tmain(int argc, _TCHAR* argv[])
{
	VectorReader myVectorReader;
	VectorRecord_t currentVector;
	
	unsigned long seconds_counter = 0;
	unsigned long steps_counter = 0;
	bool update_vector = false;
	bool update_PID_control = false;
	bool last_iteration = false;
	
	float plantState;
	float processF;
	uint16_t processValue;
	float setPointF;
	uint16_t setPoint; 
	uint16_t setPoint_old = 0;
	float effect = 0;
	
	float k_norm = 0.446f;
	float offset_norm = 48.144f;

	float tempSetting;							// Temperature setting
	bool reg_enabled;								// Heater ON/OFF

	
	
	// Log data files
	LPCSTR outDir = "output\\";
	char* fname_state_int =	    "output\\col_0.txt";
	char* fname_state_float =	"output\\col_0f.txt";
	char* fname_setting =		"output\\setting.txt";
	char* fname_p_term =		"output\\col_5.txt";
	char* fname_d_term =		"output\\col_6.txt";
	char* fname_i_term =		"output\\col_7.txt";
	char* fname_pid_output =	"output\\col_8.txt";

	if (!CreateDirectory(outDir,NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			std::cout << "Cannot create output log directory" << std::endl;
			std::cin.get();
			return 0;
		}
	}


	//-----------------------------//
	// Reading test vector file
	
	// Check arguments
	if (argc != 2)	// First argument - execution string
	{
		std::cout << "Input vector file name expected!" << std::endl;
		std::cin.get();
		return 0;
	}
	// Read test vector from file
	if (myVectorReader.ReadVectorFile((char *)argv[1]) == false)
	{
		std::cout << "Cannot parse test vector file. Press any key to exit." << std::endl;
		std::cin.get();
		return 0;
	}

	std::cout << "Test vector file OK. Starting simulation." << std::endl;

	//-----------------------------//
	// Initializing simulation
	
	// Open LOG files
	FILE *f_state_float = fopen( fname_state_float, "w" ); 
	FILE *f_state_int =	fopen( fname_state_int, "w" ); 
	FILE *f_setting = fopen( fname_setting, "w" );
	FILE *f_p_term = fopen( fname_p_term, "w" ); 
	FILE *f_d_term = fopen( fname_d_term, "w" ); 
	FILE *f_i_term = fopen( fname_i_term, "w" ); 
	FILE *f_pid_output = fopen( fname_pid_output, "w" ); 
	

	// Set ambient temperature and plant internal state
	if (!myVectorReader.StartConditions.AmbientValid)
		myVectorReader.StartConditions.Ambient = 25;
	if (!myVectorReader.StartConditions.StateValid)
		myVectorReader.StartConditions.SystemState = 25;
	initPlant((float)myVectorReader.StartConditions.Ambient, (float)myVectorReader.StartConditions.SystemState); 

	// Initialize PID controller
	plantState = (float)getPlantState();					
	processF = (plantState + offset_norm) / k_norm;	
	processF *= 4;
	//processF /= 2;
	processValue = (uint16_t)processF;
	initPID(processValue);	

	
	// Initial simulator state
	reg_enabled = false;		// heater OFF
	tempSetting = 25.0f;		// Temperature default setting
	myVectorReader.GetNextVector(&currentVector);
	
	//int32_t aaa;
	//aaa = INT32_MAX;
	//printf("%d",aaa);
	//std::cin.get();

	//-----------------------------//
	// Simulate

	while(true)
	{
		// Process time counters
		update_vector = false;
		update_PID_control = false;
		if (steps_counter % STEPS_PER_SECOND == 0)
		{
			if (seconds_counter == currentVector.TimeStamp)
			{
				update_vector = true;
			}
			if (seconds_counter % PID_CALL_INTERVAL == 0)
			{
				update_PID_control = true;
			}
			seconds_counter++;
		}
		steps_counter++;

		// Update setting using data from test vector file
		if (update_vector)
		{
			if (last_iteration)
			{
				printf("%10lu sec. Simulation finished.\n", currentVector.TimeStamp);
				break;
			}
			
			tempSetting = (float)currentVector.ForceValue;		
			reg_enabled = currentVector.ProcessEnabled;
			if (reg_enabled)
				printf("%10lu sec. New setting = %.2f\n", currentVector.TimeStamp, tempSetting);
			else
				printf("%10lu sec. New setting = %s\n", currentVector.TimeStamp, "OFF");
			
			last_iteration = !myVectorReader.GetNextVector(&currentVector);
			//std::cin.get();
		}


		// Process plant with TIMESTEP interval
		processPlant(effect);	
		
		// Process regulator 

		if (!reg_enabled)
		{
#ifdef PID_INCREMENTAL
			initPID(0);
#endif
		}
		else if (update_PID_control)
		{	
			// Calculate process value
			plantState = (float)getPlantState();					
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
#ifdef PID_INCREMENTAL
			if (setPoint != setPoint_old)
			{
				initPID(0);
				setPoint_old = setPoint;
			}
#endif
			effect = processPID(setPoint, processValue);
		}
		

		// LOG
		fprintf(f_state_float, "%f\r", getPlantState());
		fprintf(f_state_int, "%u\r", (uint16_t)getPlantState());		
		fprintf(f_setting, "%d\r", (int)tempSetting);
		fprintf(f_p_term, "%d\r", dbg_PID_p_term);
		fprintf(f_d_term, "%d\r", dbg_PID_d_term);
		fprintf(f_i_term, "%d\r", dbg_PID_i_term);
		fprintf(f_pid_output, "%d\r", dbg_PID_output);

	}

	//-------------------------------//


	
	fclose(f_state_float);
	fclose(f_state_int);
	fclose(f_setting);
	fclose(f_p_term);
	fclose(f_d_term);
	fclose(f_i_term);
	fclose(f_pid_output);
	
	std::cout << "Done. Press enter to exit." << std::endl;
	std::cin.get();

	return 0;
}

