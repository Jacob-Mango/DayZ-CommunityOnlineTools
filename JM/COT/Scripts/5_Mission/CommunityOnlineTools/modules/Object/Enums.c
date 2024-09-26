enum COT_ObjectSpawnerMode
{
	COPYLISTRAW,
	COPYLISTTYPES,
	COPYLISTEXPMARKET,
	
	CURSOR,
	PLAYER_POSITION,
	TARGET_INVENTORY,
	PLAYER_INVENTORY
};

enum COT_ESPMode
{
	COPYLISTRAW,
	COPYLISTSPAWNABLETYPES,
	COPYLISTEXPMARKET,
	
	CREATELOADOUT
};

enum COT_LoadoutSpawnMode
{
	CURSOR,
	TARGET,
	PLAYER
};

enum COT_ObjectSetupMode
{
	DEBUGSPAWN,
	CE,
	NONE
}

class COT_BloodTypes
{
	static const int UNKNOWN = GROUP_LIQUID_BLOOD;
	static const int ZERO_P = LIQUID_BLOOD_0_P;
	static const int ZERO_N = LIQUID_BLOOD_0_N;
	static const int A_P = LIQUID_BLOOD_A_P;
	static const int A_N = LIQUID_BLOOD_A_N;
	static const int B_P = LIQUID_BLOOD_B_P;
	static const int B_N = LIQUID_BLOOD_B_N;
	static const int AB_P = LIQUID_BLOOD_AB_P;
	static const int AB_N = LIQUID_BLOOD_AB_N;
};

//! LEGACY, not used
class COT_LiquidTypes
{
	static const int UNKNOWN;
	static const int WATER = LIQUID_WATER;
	static const int RIVERWATER = LIQUID_RIVERWATER;
	static const int VODKA = LIQUID_VODKA;
	static const int BEER = LIQUID_BEER;
	static const int GASOLINE = LIQUID_GASOLINE;
	static const int DIESEL = LIQUID_DIESEL;
	static const int DISINFECTANT = LIQUID_DISINFECTANT;
	static const int SOLUTION = LIQUID_SOLUTION;
};
