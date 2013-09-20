
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
using namespace std;


#include <fstream>

#include "vector_reader.h"

vector<VectorRecord_t> formedForceVector;
int VectorPtr;


bool VectorReader::GetNextVector(VectorRecord_t *rec)
{
	*rec = formedForceVector[VectorPtr++];
	if (VectorPtr < formedForceVector.size())
		return true;
	else
		return false;
}


bool VectorReader::ReadVectorFile(char *fileName)
{
	ifstream in_stream;
	char line[256];
	char delims[] = " \t";
	vector<char*> words;
	VectorRecord_t newRecord;
	bool result = true;

	formedForceVector.clear();
	VectorPtr = 0;
	VectorReader::StartConditions.AmbientValid = false;
	VectorReader::StartConditions.StateValid = false;

	try 
	{
		cout << "Reading file: " << fileName << endl;

		in_stream.open(fileName);
		if (in_stream.is_open()) 
		{
			while( (in_stream.rdstate() & (std::ifstream::failbit | std::ifstream::eofbit)) == 0 )
			{
				// Parse vector file record

				in_stream.getline(line,256,'\n');
				//cout << line << endl;
				words = SplitByDelims(line, delims);		
				//for ( int i = 0; i < words.size(); ++i)
				//	cout << words[i] << "\n";
				if (words.size() < 2)
				{
					cout << "Test vector file record error. Scipping this record." << endl;
					continue;
				}

				// Look for start conditions
				if (strcmp(words[0], ".ambient") == 0)
				{
					VectorReader::StartConditions.Ambient = strtol((const char*)words[1], NULL, 10);
					VectorReader::StartConditions.AmbientValid = true;
				}
				else if (strcmp(words[0], ".state") == 0)
				{
					VectorReader::StartConditions.SystemState = strtol((const char*)words[1], NULL, 10);
					VectorReader::StartConditions.StateValid = true;
				}
				else
				{
					newRecord.TimeStamp = strtoul((const char*)words[0], NULL, 10);
					//cout << "Timestamp = " << newRecord.TimeStamp << endl;
					if (strcmp(words[1], "OFF") != 0)
					{
						newRecord.ForceValue = strtol((const char*)words[1], NULL, 10);
						newRecord.ProcessEnabled = true;
					}
					else
					{
						newRecord.ForceValue = 0;
						newRecord.ProcessEnabled = false;
					}
					formedForceVector.push_back(newRecord); 
				}
			}
			cout << "Total read: " << formedForceVector.size() << " records" << endl;
		}
		else 
		{
			std::cout << "Cannot open file \n";
			result = false;
		}
	}
	catch (exception& e)
	{
		cout << e.what() << endl;
	}

	if (in_stream.is_open())
		in_stream.close();

	if (formedForceVector.size() < 2)
	{
		std::cout << "Vector file must contain at least 2 records\n";
		result = false;
	}

	return result;
}




vector<char*> VectorReader::SplitByDelims(char source[], char delims[] )
{
	vector<char*> vec_String_Lines;
	char *token = strtok( source, delims );

	while( token != NULL )
	{
      vec_String_Lines.push_back(token);
      token = strtok( NULL, delims );
	}

	return vec_String_Lines;
}
