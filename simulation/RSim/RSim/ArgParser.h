

typedef struct
{
	char * option_switch;
	char * option_value;
} option_pair_t;


class ArgParser
{
public:
	int Parse(int argc, char* argv[]);
	option_pair_t* GetOption(char *option_switch);
	char* GetOptionValue(char *option_switch);
	void PrintOptions(void);
private:
};


