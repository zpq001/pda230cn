// adc_test1.cpp: определяет точку входа для консольного приложения.
//



#include "stdint.h"
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <iostream>


extern "C" {
	#include "adc_func.h"
}


int _tmain(int argc, _TCHAR* argv[])
{
	cp.cpoint1 = 22;
	cp.cpoint1_adc = 4*195;
	cp.cpoint2 = 120;
	cp.cpoint2_adc = 4*401;

	//cp.cpoint1 = 20;
	//cp.cpoint1_adc = 200;
	//cp.cpoint2 = 320;
	//cp.cpoint2_adc = 6200;

	//cp.cpoint1 = 20;
	//cp.cpoint1_adc = 220;
	//cp.cpoint2 = 320;
	//cp.cpoint2_adc = 1720;

	//cp.cpoint1 = 20;
	//cp.cpoint1_adc = 200;
	//cp.cpoint2 = 194;
	//cp.cpoint2_adc = 2200;
	
	const int plot_start_value_celsius	= 10;
	const int plot_end_value_celsius	= 350;
	const int plot_celsius_increment = 1;
	
	const int plot_start_value_adc	= 80;
	const int plot_end_value_adc	= 6500;
	const int plot_adc_increment = 1;

	int plot_arg;
	float temp_f;
	int err;

	char tmp_buf_char[100];
	char *output_dir = "output\\";
	//------------------------//

	calculateCoeffs();
	calculateCoeffs_float();

	printf("Coefficients are:\n");
	printf("k_norm:            %d\n",k_norm);
	printf("offset_norm:       %d\n",offset_norm);
	printf("k_norm_float:      %f\n",k_norm_f);
	printf("offset_norm_float: %f\n",offset_norm_f);

	//------------------------//
	if (!CreateDirectory((LPCWSTR)output_dir,NULL))
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			std::cout << "Cannot create output log directory" << std::endl;
			std::cin.get();
			return 0;
		}
	}

	// LOG files
	FILE *f1_adc;
	FILE *f1_celsius;
	FILE *f2_adc;
	FILE *f2_adc_float;
	FILE *f2_celsius;
	
	//---------------------------------------------------//
	// Function = Celsius(ADC)
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "f1_adc.txt");
	err = fopen_s( &f1_adc, tmp_buf_char, "w" );

	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "f1_celsius.txt");
	err = fopen_s( &f1_celsius, tmp_buf_char, "w" );

	//------------------------//
	// Print Celsius(ADC) plot
	for (plot_arg = plot_start_value_adc; plot_arg != plot_end_value_adc; plot_arg += plot_adc_increment)
	{
		fprintf(f1_adc,"%d\r",plot_arg);
		temp_f = conv_ADC_to_Celsius(plot_arg);
		fprintf(f1_celsius,"%f\r",temp_f);
	}
	_fcloseall();
	
	//---------------------------------------------------//
	// Function = ADC(Celsius)
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "f2_adc.txt");
	err = fopen_s( &f2_adc, tmp_buf_char, "w" );
    
	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "f2_adc_float.txt");
	err = fopen_s( &f2_adc_float, tmp_buf_char, "w" );

	sprintf_s(tmp_buf_char, 100, "%s%s", output_dir, "f2_celsius.txt");
	err = fopen_s( &f2_celsius, tmp_buf_char, "w" );

	//------------------------//
	// Print ADC(Celsius) plot
	for (plot_arg = plot_start_value_celsius; plot_arg != plot_end_value_celsius; plot_arg += plot_celsius_increment)
	{
		fprintf(f2_celsius,"%d\r",plot_arg);
		temp_f = conv_Celsius_to_ADC(plot_arg);
		fprintf(f2_adc,"%f\r",temp_f);
		temp_f = conv_Celsius_to_ADC_float(plot_arg);
		fprintf(f2_adc_float,"%f\r",temp_f);
	}
	_fcloseall();
	
	std::cin.get();

	return 0;
}

