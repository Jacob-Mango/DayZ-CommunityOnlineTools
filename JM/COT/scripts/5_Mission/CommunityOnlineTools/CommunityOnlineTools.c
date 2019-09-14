class CommunityOnlineTools: CommunityOnlineToolsBase
{
	void CommunityOnlineTools()
	{
		//Print("CommunityOnlineTools::CommunityOnlineTools");

		GetRPCManager().AddRPC( "COT", "RefreshPlayers", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT", "RemoveClient", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "SetClientInstance", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT", "UpdatePlayer", this, SingeplayerExecutionType.Client );
		
		GetRPCManager().AddRPC( "COT", "UpdateRole", this, SingeplayerExecutionType.Client );

		GetPermissionsManager().RegisterPermission( "Admin.Player.Read" );
		GetPermissionsManager().RegisterPermission( "Admin.Roles.Update" );
	}

	void ~CommunityOnlineTools()
	{
		//Print("CommunityOnlineTools::~CommunityOnlineTools");
	}
	
	override void OnStart()
	{
		super.OnStart();
	}

	override void OnFinish()
	{
		super.OnFinish();

		SetOpen( false );
	}

	override void OnLoaded()
	{
		if ( IsMissionClient() )
		{
			GetRPCManager().SendRPC( "COT", "RefreshPlayers", new Param, true );
		}

		super.OnLoaded();
	}

	override void OnUpdate( float timeslice )
	{
		super.OnUpdate( timeslice );
	}

	void RefreshPlayers( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			array< JMPlayerInstance > players = GetPermissionsManager().GetPlayers();

			for ( int i = 0; i < players.Count(); i++ )
			{
				players[i].UpdatePlayerData();
				players[i].Serialize();

				GetRPCManager().SendRPC( "COT", "UpdatePlayer", new Param3< ref JMPlayerInformation, PlayerIdentity, PlayerBase >( players[i].Data, players[i].IdentityPlayer, players[i].PlayerObject ), false, sender );
			}
		}
	}

	void RemoveClient( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Client )
		{
			if ( GetGame().IsMultiplayer() )
			{
				ref Param1< string > data;
				if ( !ctx.Read( data ) )
					return;
				
				JMPlayerInstance instance;
				GetPermissionsManager().OnClientDisconnected( data.param1, instance );

				RemoveSelectedPlayer( data.param1 );
			}
		}
	}

	void UpdatePlayer( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Player.Read", sender ) )
				return;

			ref Param1< string > data;
			if ( !ctx.Read( data ) )
				return;

			JMPlayerInstance player = GetPermissionsManager().GetPlayer( data.param1 );
			if ( !player )
				return;

			player.UpdatePlayerData();
			player.Serialize();

			GetRPCManager().SendRPC( "COT", "UpdatePlayer", new Param3< ref JMPlayerInformation, PlayerIdentity, PlayerBase >( player.Data, player.IdentityPlayer, player.PlayerObject ), false, sender );
		}

		if ( type == CallType.Client )
		{
			ref Param3< ref JMPlayerInformation, PlayerIdentity, PlayerBase > cdata;
			if ( !ctx.Read( cdata ) )
				return;

			GetPermissionsManager().UpdatePlayer( cdata.param1, cdata.param2, cdata.param3 );
		}
	}

	void SetClientInstance( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( type == CallType.Client )
		{
			ref Param4< string, ref JMPlayerInformation, PlayerIdentity, PlayerBase > data;
			if ( !ctx.Read( data ) )
				return;

			GetPermissionsManager().UpdatePlayer( data.param2, data.param3, data.param4 );

			GetPermissionsManager().SetClientGUID( data.param1 );

			GetModuleManager().OnClientPermissionsUpdated();
		}
	}

	void UpdateRole( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		ref Param2< string, ref array< string > > data;
		if ( !ctx.Read( data ) )
			return;

		ref array< string > arr = new array< string >;
		arr.Copy( data.param2 );

		ref JMRole role = NULL;

		if ( type == CallType.Server )
		{
			if ( !GetPermissionsManager().HasPermission( "Admin.Roles.Update", sender ) )
				return;

			GetPermissionsManager().Roles.Find( data.param1, role );

			if ( role )
			{
				role.ClearPermissions();

				role.SerializedData = arr;

				role.Deserialize();
			} else 
			{
				role = GetPermissionsManager().LoadRole( data.param1, arr );
			}

			role.Serialize();
				
			GetRPCManager().SendRPC( "COT", "UpdateRole", new Param2< string, ref array< string > >( role.Name, role.SerializedData ), false );
		}

		if ( type == CallType.Client )
		{
			GetPermissionsManager().Roles.Find( data.param1, role );

			if ( role )
			{
				role.ClearPermissions();

				role.SerializedData = arr;

				role.Deserialize();
			} else 
			{
				GetPermissionsManager().LoadRole( data.param1, arr );
			}

			GetModuleManager().OnClientPermissionsUpdated();
		}
	}
}

CommunityOnlineTools GetCommunityOnlineTools()
{
    return CommunityOnlineTools.Cast( g_cotBase );
}