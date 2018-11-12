// STORE ALL PLAYERS DATA HERE FOR OUTSIDE NETWORK BUBBLE!

class PlayerData
{
    string SName;
    string SGUID;

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
}