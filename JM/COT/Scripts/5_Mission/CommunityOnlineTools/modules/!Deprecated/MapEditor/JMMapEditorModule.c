class JMMapEditorModule: JMModuleBase
{
	protected bool m_InEditor;

	protected ref JMMapEditorForm m_Menu;

	void JMMapEditorModule()
	{
		GetPermissionsManager().RegisterPermission( "MapEditor.EnterEditor" );
		GetPermissionsManager().RegisterPermission( "MapEditor.LeaveEditor" );
		GetPermissionsManager().RegisterPermission( "MapEditor.Transform.Position" );
		
		GetRPCManager().AddRPC( "COT_MapEditor", "EnterEditor", this, SingeplayerExecutionType.Client );
		GetRPCManager().AddRPC( "COT_MapEditor", "LeaveEditor", this, SingeplayerExecutionType.Client );
	}

	override void EnableUpdate()
	{
	}
	
	void SetTransform( CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "MapEditor.Transform.Position", senderRPC ) )
			return;

		Param2< vector, vector > data;
		if ( !ctx.Read( data ) ) return;
 
		if ( target == NULL )
			return;

		if ( type == CallType.Server )
		{
			//target.SetOrigin( data.param1 );
			target.SetPosition( data.param1 );
			target.SetOrientation( data.param2 );
			//target.Update();
		}
	}
	
	void EnterEditor( CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{			
		if ( !GetPermissionsManager().HasPermission( "MapEditor.EnterEditor", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			PlayerBase.Cast(senderRPC.GetPlayer()).COT_TempDisableOnSelectPlayer();

			vector position = Vector( 0, 0, 0 );

			if ( target )
			{
				position = target.GetPosition();
			}

			GetGame().SelectPlayer( senderRPC, NULL );

			PlayerBase human = PlayerBase.Cast( target );

			if ( human )
			{
				// human.Save();
				// human.Delete();

				position = human.GetBonePositionWS( human.GetBoneIndexByName("Head") );
			}

			GetGame().SelectSpectator( senderRPC, "JMCinematicCamera", position );

			GetRPCManager().SendRPC( "COT_MapEditor", "EnterEditor", new Param, true, senderRPC );

			GetCommunityOnlineToolsBase().Log( senderRPC, "Entered the Map Editor");
		}

		if ( type == CallType.Client )
		{
			GetPlayer().COT_TempDisableOnSelectPlayer();

			if ( GetGame().IsMultiplayer() )
			{
				CurrentActiveCamera = JMCameraBase.Cast( Camera.GetCurrentCamera() );
			} else 
			{
				CurrentActiveCamera = JMCameraBase.Cast( GetGame().CreateObject( "JMCinematicCamera", target.GetPosition(), false ) );
			}

			if ( CurrentActiveCamera )
			{
				m_InEditor = true;

				m_Menu.Show();

				CurrentActiveCamera.SetActive( true );
				
				if ( GetPlayer() )
				{
					GetPlayer().GetInputController().SetDisabled( true );
				}
			}
		}
	}
	
	void LeaveEditor( CallType type, ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{			
		if ( !GetPermissionsManager().HasPermission( "MapEditor.LeaveEditor", senderRPC ) )
			return;

		bool cont = false;

		if ( type == CallType.Server )
		{
			GetGame().SelectPlayer( senderRPC, target );

			if ( GetGame().IsMultiplayer() )
			{
				GetRPCManager().SendRPC( "COT_MapEditor", "LeaveEditor", new Param, true, senderRPC );
			} 

			GetCommunityOnlineToolsBase().Log( senderRPC, "Left the Map Editor");
		}

		if ( type == CallType.Client )
		{
			if ( !GetGame().IsMultiplayer() )
			{
				GetGame().SelectPlayer( senderRPC, target );
			}

			CurrentActiveCamera.SetActive( false );
			CurrentActiveCamera = NULL;
			
			PPEffects.ResetDOFOverride();

			if ( GetPlayer() )
			{
				GetPlayer().GetInputController().SetDisabled( false );
			}

			m_InEditor = false;

			m_Menu.Hide();
		}
	}

	override void RegisterKeyMouseBindings() 
	{
		super.RegisterKeyMouseBindings();
		
		Bind( new JMModuleBinding( "ToggleEditor",		"UACOTToggleMapEditor",	true 	) );
	}
	
	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
		
		if ( GetGame().IsClient() )
		{
			if ( m_Menu == NULL )
			{
				m_Menu = new JMMapEditorForm( this );
				m_Menu.Init();
			}
		}
	}

	void ToggleEditor( UAInput input )
	{
		if ( !( input.LocalPress() ) )
			return;
			
		if ( CurrentActiveCamera || m_InEditor )
		{
			GetRPCManager().SendRPC( "COT_MapEditor", "LeaveEditor", new Param, false, NULL, GetPlayer() );
		} else 
		{
			GetRPCManager().SendRPC( "COT_MapEditor", "EnterEditor", new Param, false, NULL, GetPlayer() );
		}
	}
};
