class COTManagerModule: EditorModule
{
	protected ref COTManagerSettings settings;

	void COTManagerModule()
	{
		GetRPCManager().AddRPC( "COT_Manager", "LoadData", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "COT.Apply" );
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "COT.View" );
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		if ( GetGame().IsClient() )
			GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param, true );
	}

	override void ReloadSettings()
	{
		super.ReloadSettings();

		settings = COTManagerSettings.Load();
	}

	override void OnMissionFinish()
	{
		super.OnMissionFinish();

		if ( GetGame().IsServer() )
			settings.Save();
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/gui/layouts/COTManager/COTManagerMenu.layout";
	}

	COTManagerSettings GetSettings()
	{
		return settings;
	}
	
	void LoadData( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		Param1< string > data;
		if( type == CallType.Server )
		{
			if ( ctx.Read( data ) )
			{
				if ( !GetPermissionsManager().HasPermission( "COT.Apply", sender ) )
				{
					return;
				}

				delete settings;
				JsonFileLoader< COTManagerSettings >.JsonLoadData( data.param1, settings );
				settings.Save();
			} else 
			{
				GetRPCManager().SendRPC( "COT_Manager", "LoadData", new Param1< string >( JsonFileLoader< COTManagerSettings >.JsonMakeData( settings ) ), true );
			}
		}

		if( type == CallType.Client )
		{
			if ( !ctx.Read( data ) ) return;

			JsonFileLoader< COTManagerSettings >.JsonLoadData( data.param1, settings );

			if ( form && form.IsVisible() )
			{
				form.OnShow();
			}
		}
	}
}