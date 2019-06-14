// STORE ALL PLAYERS DATA HERE FOR OUTSIDE NETWORK BUBBLE!

class PlayerData
{
	string SName;
	string SGUID;
	string SSteam64ID;

	int IPingMax;
	int IPingMin;
	int IPingAvg;

	ref array< string > ARoles;
	ref array< string > APermissions;

	vector VPosition;
	vector VDirection;
	vector VOrientation;

	float FHealth;
	float FBlood;
	float FShock;

	int IBloodStatType;

	float FEnergy;
	float FWater;

	float FHeatComfort;

	float FWet;
	float FTremor;
	float FStamina;

	int Kills;
	int TotalKills;
	
	int ILifeSpanState;
	bool BBloodyHands;
	bool BGodMode;
	bool BInvisibility;

	void PlayerData()
	{
		ARoles = new ref array< string >;
		APermissions = new ref array< string >;
	}

	void Load( PlayerBase player )
	{
		if ( player == NULL )
			return;

		VPosition = player.GetPosition();
		VDirection = player.GetDirection();
		VOrientation = player.GetOrientation();
	
		FHealth = player.GetHealth( "GlobalHealth","Health" );
		FBlood = player.GetHealth( "GlobalHealth", "Blood" );
		FShock = player.GetHealth( "GlobalHealth", "Shock" );
		IBloodStatType = player.GetStatBloodType().Get();
		FEnergy = player.GetStatEnergy().Get();
		FWater = player.GetStatWater().Get();
		FHeatComfort = player.GetStatHeatComfort().Get();
		FWet = player.GetStatWet().Get();
		FTremor = player.GetStatTremor().Get();
		FStamina = player.GetStatStamina().Get();
		ILifeSpanState = player.GetLifeSpanState();
		BBloodyHands = player.HasBloodyHands();
		BGodMode = player.m_HasGodMode;
		BInvisibility = player.m_IsInvisible;
	}
}