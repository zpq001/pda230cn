
#include <iostream>
#include <stdlib.h>

#include <vector>
#include "ArgParser.h"







std::vector<option_pair_t> option_pairs;

// Parses input argument string
// All words starting with '-' are interpreted as switches
// A single word next to switch is interpreted as parameter (value)
// Function searches for switches and creates list of such pairs

int ArgParser::Parse(int argc, char* argv[])
{
	int i = 0;
	option_pair_t new_option_record;
	while(i < argc)
	{
		// Check if analyzed word is a switch
		if (argv[i][0] == '-')
		{
			new_option_record.option_switch = argv[i];
			new_option_record.option_value = NULL;
			// Check word next to discovered switch
			if (i+1 < argc)
			{
				if (argv[i+1][0] != '-')
				{
					new_option_record.option_value = argv[i+1];
					i++;
				}
				else
				{
					// Next word is a switch too. Skip it
				}
			}
			// Add option pair to the list. Note that option_value field may be NULL
			option_pairs.push_back(new_option_record);
		}
		i++;
	}
	return 0;
}


// Returns an option pair with specified switch
// Returns null if not found

option_pair_t* ArgParser::GetOption(char *option_switch)
{
	option_pair_t *option_p;
	for (unsigned int i=0; i<option_pairs.size();i++)
	{
		option_p = &option_pairs[i];
		if (strcmp(option_p->option_switch, option_switch) == 0)
			return option_p;
	}

	return NULL;
}

char* ArgParser::GetOptionValue(char *option_switch)
{
	option_pair_t *option_p;
	for (unsigned int i=0; i<option_pairs.size();i++)
	{
		option_p = &option_pairs[i];
		if (strcmp(option_p->option_switch, option_switch) == 0)
			return option_p->option_value;
	}

	return NULL;
}


// Prints all switches and values (if present)
// Purpose: debug
void ArgParser::PrintOptions(void)
{
	std::cout << "Printing command line arguments" << std::endl;
	for (unsigned int i=0;i<option_pairs.size();i++)
	{
		std::cout << option_pairs[i].option_switch << "    \t";
		if (option_pairs[i].option_value)
			std::cout << option_pairs[i].option_value;
		std::cout << "\n";
	}
	std::cout << "Done!" << std::endl;
}