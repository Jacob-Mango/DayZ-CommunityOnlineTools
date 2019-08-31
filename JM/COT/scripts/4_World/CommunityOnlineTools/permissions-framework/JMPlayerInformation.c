class JMPlayerInformation
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

	void JMPlayerInformation()
	{
		ARoles = new array< string >;
		APermissions = new array< string >;
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
		BGodMode = player.HasGodMode();
		BInvisibility = player.IsInvisible();
	}

	void Copy( JMPlayerInformation data, bool isClient )
	{
		if ( isClient )
		{
			SName	    	= "" + data.SName;
			SGUID	    	= "" + data.SGUID;
			SSteam64ID		= "" + data.SSteam64ID;

			IPingMax		= data.IPingMax;
			IPingMin		= data.IPingMin;
			IPingAvg		= data.IPingAvg;
		}

		ARoles.Copy( data.ARoles );
		APermissions.Copy( data.APermissions );

		VPosition		= data.VPosition;
		VDirection		= data.VDirection;
		VOrientation	= data.VOrientation;

		FHealth	    	= data.FHealth;
		FBlood	    	= data.FBlood;
		FShock	    	= data.FShock;

		IBloodStatType  = data.IBloodStatType;

		FEnergy	    	= data.FEnergy;
		FWater		    = data.FWater;

		FHeatComfort    = data.FHeatComfort;

		FWet	       	= data.FWater;
		FTremor         = data.FTremor;
		FStamina		= data.FStamina;

		Kills		    = data.Kills;
		TotalKills		= data.TotalKills;

		ILifeSpanState	= data.ILifeSpanState;
		BBloodyHands	= data.BBloodyHands;
		BGodMode		= data.BGodMode;
		BInvisibility	= data.BInvisibility;
	}

	void DebugPrint()
	{
		Print( "-------------------------------" );
		Print( SName );
		Print( SGUID );
		Print( SSteam64ID );
		Print( IPingMax );
		Print( IPingMin );
		Print( IPingAvg );
		Print( ARoles );
		Print( APermissions	);
		Print( VPosition );
		Print( VDirection );
		Print( VOrientation	);
		Print( FHealth );
		Print( FBlood );
		Print( FShock );
		Print( IBloodStatType );
		Print( FEnergy );
		Print( FWater );
		Print( FHeatComfort );
		Print( FWet );
		Print( FTremor );
		Print( FStamina );
		Print( Kills );
		Print( TotalKills );
		Print( ILifeSpanState );
		Print( BBloodyHands );
		Print( BGodMode );
		Print( BInvisibility );
		Print( "-------------------------------" );
	}
}