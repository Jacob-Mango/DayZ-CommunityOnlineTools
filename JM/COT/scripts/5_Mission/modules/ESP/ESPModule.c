class ESPModule: EditorModule
{
    protected float m_ESPRadius;

    protected bool m_CanViewPlayers;
    protected bool m_CanViewBaseBuilding;
    protected bool m_CanViewVehicles;

    void ESPModule()
    {
        m_ESPRadius = 200;

        //GetRPCManager().AddRPC( "ESP_", "", this, SingeplayerExecutionType.Server );

        GetPermissionsManager().RegisterPermission( "ESP.View.Player" );
        GetPermissionsManager().RegisterPermission( "ESP.View.BaseBuilding" );
        GetPermissionsManager().RegisterPermission( "ESP.View.Vehicles" );
        GetPermissionsManager().RegisterPermission( "ESP.View" );
    }

    override bool HasAccess()
    {
        return GetPermissionsManager().HasPermission( "ESP.View" );
    }

    override string GetLayoutRoot()
    {
        return "JM/COT/gui/layouts/Object/ObjectMenu.layout";
    }

    override void OnClientPermissionsUpdated()
    {
        m_CanViewPlayers = GetPermissionsManager().HasPermission( "ESP.View.Player" );
        m_CanViewBaseBuilding = GetPermissionsManager().HasPermission( "ESP.View.BaseBuilding" );
        m_CanViewVehicles = GetPermissionsManager().HasPermission( "ESP.View.Vehicles" );
    }

    void UpdateESP()
    {
		ref array<Object> objects = new array<Object>;
        GetGame().GetObjectsAtPosition3D( GetCurrentPosition(), m_ESPRadius, objects, NULL );

        Object currentObj;

        for (int i = 0; i < objects.Count(); ++i)
		{
			currentObj = Object.Cast( objects.Get(i) );

			if ( m_CanViewPlayers && GetGame().ObjectIsKindOf( obj, "SurvivorBase" ) )
			{

            }
        }
    }
}