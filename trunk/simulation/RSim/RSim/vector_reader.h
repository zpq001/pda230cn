

#include <string>
#include <vector>
using namespace std;




typedef struct
{
	unsigned long TimeStamp;
	int ForceValue;
	bool ProcessEnabled;
} VectorRecord_t;

typedef struct
{
	int Ambient;
	int SystemState;
	bool AmbientValid;
	bool StateValid;
} StartCondition_t;



class VectorReader
{
public:
	bool ReadVectorFile(char *fileName);	
	bool GetNextVector(VectorRecord_t *rec);
	StartCondition_t StartConditions;
private:
	vector<char*> SplitByDelims(char source[], char delims[] );
	
};


