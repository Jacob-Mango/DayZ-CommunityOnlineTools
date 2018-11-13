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

    int IBloodStatType;

    float FTemperature;
    float FEnergy;
    float FWater;
    float FShock;

    float FStomachWater;
    float FStomachEnergy;

    float FHeatComfort;
    float FStomachSolid;

    float FWet;
    float FTremor;
    
    float FStamina;
    
    float FLastShaved;
    float FBloodyHands;
    
    float FPlayTime;

    void PlayerData()
    {
        ARoles = new ref array< string >;
        APermissions = new ref array< string >;
    }

    static void Load( ref PlayerData data, Man man )
    {
        //data.SModel = man.GetType();

        data.VPosition = man.GetPosition();
        data.VDirection = man.GetDirection();
        data.VOrientation = man.GetOrientation();
        
        data.FHealth = man.GetHealth("","");

        PlayerBase player = PlayerBase.Cast( man );

        if ( player == NULL ) return;

        data.FBlood = player.GetHealth("GlobalHealth", "Blood");
        data.IBloodStatType = player.GetStatBloodType().Get();
        //data.FTemperature = player.GetStatTemperature().Get();
        //data.FShock = player.GetStatShock().Get();
        data.FEnergy = player.GetStatEnergy().Get();
        data.FWater = player.GetStatWater().Get();
        data.FStomachWater = player.GetStatStomachWater().Get();
        data.FStomachEnergy = player.GetStatStomachEnergy().Get();
        data.FHeatComfort = player.GetStatHeatComfort().Get();
        // data.FStomachSolid = player.GetStatStomachSolid().Get();
        data.FWet = player.GetStatWet().Get();
        data.FTremor = player.GetStatTremor().Get();
        data.FStamina = player.GetStatStamina().Get();
        data.FLastShaved = player.GetLastShavedSeconds();
        data.FBloodyHands = player.HasBloodyHands();
        data.FPlayTime = player.StatGet("playtime");
    }
}