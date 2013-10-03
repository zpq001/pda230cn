// RSim.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <windows.h>
#include <iostream>

#include <string>

#include "vector_reader.h"
#include "ArgParser.h"

#include "stdafx.h"
#include "stdint.h"
#include "simulation.h"
#include "plant.h"
extern "C" {
	#include "pid_controller.h"
}


enum SimulationMode {SIM_PLANT_STEP_RESPONSE, SIM_NORMAL};



int _tmain(int argc, _TCHAR* argv[])
{
	VectorReader myVectorReader;
	VectorRecord_t currentVector;
	ArgParser myArgParser;

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

	
	char *input_fname;
	char *output_dir;
	char *tmp_arg_str;
	int simulation_mode;

	char tmp_buf_char[100];

	//--------------------------------------------//
	// Command line arguments parsing

	myArgParser.Parse(argc, (char **)argv);
	 //myArgParser.PrintOptions();
	 //std::cin.get();
	 //return 0;

	if (!(input_fname = myArgParser.GetOptionValue("-input")))
	{
		std::cout << "Expected test vector file (-input <file>)" << std::endl;
		std::cin.get();
		return 0;
	}
	
	if (!(output_dir = myArgParser.GetOptionValue("-outdir")))
	{
		std::cout << "Expected output directory (-outdir <directory>)" << std::endl;
		std::cin.get();
		return 0;
	}

	if (!(tmp_arg_str = myArgParser.GetOptionValue("-mode")))
	{
		std::cout << "Expected simulation mode (-mode <PLANT_STEP / NORMAL>)" << std::endl;
		std::cin.get();
		return 0;
	}
	simulation_mode = (strcmp(tmp_arg_str, "PLANT_STEP") == 0) ? SIM_PLANT_STEP_RESPONSE : SIM_NORMAL;

	//--------------------------------------------//



	//-----------------------------//
	// Reading test vector file
	if (myVectorReader.ReadVectorFile(input_fname) == false)
	{
		std::cout << "Cannot read test vector file. Press any key to exit." << std::endl;
		std::cin.get();
		return 0;
	}

	std::cout << "Test vector file OK. Starting simulation." << std::endl;


	//-----------------------------//
	// Initializing simulation
	
	if (!CreateDirectory(output_dir,NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			std::cout << "Cannot create output log directory" << std::endl;
			std::cin.get();
			return 0;
		}
	}

	// Open LOG files
	FILE *f_state_float;
	FILE *f_state_int;
	FILE *f_setting;
	FILE *f_p_term;
	FILE *f_d_term;
	FILE *f_i_term;
	FILE *f_pid_output;

	// Create log data files:

	// Plant state float
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_0f.txt");
	fopen_s( &f_state_float, tmp_buf_char, "w" ); 
	// Plant state integer
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_0.txt");
	fopen_s( &f_state_int, tmp_buf_char, "w" ); 
	// Set value
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "setting.txt");
	fopen_s( &f_setting, tmp_buf_char, "w" ); 
	// P-term of PID controller
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_5.txt");
	fopen_s( &f_p_term, tmp_buf_char, "w" ); 
	// D-term of PID controller
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_6.txt");
	fopen_s( &f_d_term, tmp_buf_char, "w" ); 
	// I-term of PID controller
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_7.txt");
	fopen_s( &f_i_term, tmp_buf_char, "w" ); 
	// Output of PID controller
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "col_8.txt");
	fopen_s( &f_pid_output, tmp_buf_char, "w" ); 
	

	// Set ambient temperature and plant internal state
	if (!myVectorReader.StartConditions.AmbientValid)
		myVectorReader.StartConditions.Ambient = 25;
	if (!myVectorReader.StartConditions.StateValid)
		myVectorReader.StartConditions.SystemState = 25;
	initPlant((float)myVectorReader.StartConditions.Ambient, (float)myVectorReader.StartConditions.SystemState); 
	processPlant(0);

	// Initialize PID controller
	// Calculate process value
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
		if (simulation_mode == SIM_PLANT_STEP_RESPONSE)
		{
				if (reg_enabled)
					effect = 100;
				else
					effect = 0;
				dbg_PID_output = (int16_t)effect;
		}
		else
		{
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

