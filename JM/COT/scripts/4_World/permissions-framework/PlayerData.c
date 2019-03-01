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
    bool BGodMode

    void PlayerData()
    {
        ARoles = new ref array< string >;
        APermissions = new ref array< string >;
    }

    static void Load( out PlayerData data, ref PlayerBase player )
    {
        data.VPosition = player.GetPosition();
        data.VDirection = player.GetDirection();
        data.VOrientation = player.GetOrientation();
        
        data.FHealth = player.GetHealth( "GlobalHealth","Health" );
        data.FBlood = player.GetHealth( "GlobalHealth", "Blood" );
        data.FShock = player.GetHealth( "GlobalHealth", "Shock" );
        data.IBloodStatType = player.GetStatBloodType().Get();
        data.FEnergy = player.GetStatEnergy().Get();
        data.FWater = player.GetStatWater().Get();
        data.FHeatComfort = player.GetStatHeatComfort().Get();
        data.FWet = player.GetStatWet().Get();
        data.FTremor = player.GetStatTremor().Get();
        data.FStamina = player.GetStatStamina().Get();
        data.ILifeSpanState = player.GetLifeSpanState();
        data.BBloodyHands = player.HasBloodyHands();
        data.BGodMode = player.m_HasGodeMode;
    }
}