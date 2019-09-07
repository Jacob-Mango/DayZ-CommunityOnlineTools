class JMMapEditorModule: JMModuleBase
{
	protected bool m_InEditor;

	protected ref JMMapEditorForm m_Menu;

	void JMMapEditorModule()
	{
		GetRPCManager().AddRPC( "COT_MapEditor", "EnterEditor", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "COT_MapEditor", "LeaveEditor", this, SingeplayerExecutionType.Server );

		GetRPCManager().AddRPC( "COT_MapEditor", "SetTransform", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "MapEditor.EnterEditor" );
		GetPermissionsManager().RegisterPermission( "MapEditor.LeaveEditor" );
		GetPermissionsManager().RegisterPermission( "MapEditor.Transform.Position" );
	}
	
	void SetTransform( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "MapEditor.Transform.Position", sender ) )
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

			//SendAdminNotification( sender, NULL, target.GetDisplayName() + " has been given the position " + VectorToString( data.param1, 1 ) );
		}
	}
	
	void EnterEditor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{			
		if ( !GetPermissionsManager().HasPermission( "MapEditor.EnterEditor", sender ) )
			return;

		if ( type == CallType.Server )
		{
			vector position = Vector( 0, 0, 0 );

			if ( target )
			{
				position = target.GetPosition();
			}

			GetGame().SelectPlayer( sender, NULL );

			PlayerBase human = PlayerBase.Cast( target );

			if ( human )
			{
				// human.Save();
				// human.Delete();

				position = human.GetBonePositionWS( human.GetBoneIndexByName("Head") );
			}

			GetGame().SelectSpectator( sender, "JMCinematicCamera", position );

			GetRPCManager().SendRPC( "COT_MapEditor", "EnterEditor", new Param, true, sender );

			GetCommunityOnlineToolsBase().Log( sender, "Entered the Map Editor");
		}

		if ( type == CallType.Client )
		{
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
	
	void LeaveEditor( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity sender, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "CameraTools.LeaveCamera", sender ) )
			return;
			
		if ( !GetPermissionsManager().HasPermission( "MapEditor.LeaveEditor", sender ) )
			return;

		bool cont = false;

		if ( type == CallType.Server )
		{
			GetGame().SelectPlayer( sender, target );

			if ( GetGame().IsMultiplayer() )
			{
				GetRPCManager().SendRPC( "COT_MapEditor", "LeaveEditor", new Param, true, sender );
			} 

			GetCommunityOnlineToolsBase().Log( sender, "Left the Map Editor");
		}

		if ( type == CallType.Client )
		{
			if ( !GetGame().IsMultiplayer() )
			{
				GetGame().SelectPlayer( sender, target );
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
		RegisterKeyMouseBinding( new JMModuleBinding( "ToggleEditor",		"UAMapEditorModuleToggleEditor",	true 	) );
	}
	
	override void OnMissionLoaded()
	{
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

}