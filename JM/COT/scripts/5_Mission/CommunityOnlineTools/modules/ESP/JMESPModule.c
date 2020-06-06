enum JMESPModuleRPC
{
	INVALID = 10300,
	Log,
	FullMap,
	SetPosition,
	SetOrientation,
	SetHealth,
	COUNT
};

class JMESPModule: JMRenderableModuleBase
{
	private autoptr array< ref JMESPMeta > m_ActiveESPObjects;

	private autoptr array< ref JMESPMeta > m_ESPToCreate;
	private autoptr array< JMESPMeta > m_ESPToDestroy;

	private autoptr map< Object, JMESPMeta > m_MappedESPObjects;

	private autoptr array< ref JMESPViewType > m_ViewTypes;

	private bool m_IsCreatingWidgets;
	private bool m_IsDestroyingWidgets;

	string Filter;

	float ESPRadius;
	float ESPUpdateTime;
	bool ESPIsUpdating;

	bool IsShowing;
	bool ToDestroy;

	void JMESPModule()
	{
		m_ActiveESPObjects = new array< ref JMESPMeta >;

		m_ESPToCreate = new array< ref JMESPMeta >;
		m_ESPToDestroy = new array< JMESPMeta >;

		m_MappedESPObjects = new map< Object, JMESPMeta >;

		m_ViewTypes = new array< ref JMESPViewType >;

		ESPRadius = 200;

		ESPUpdateTime = 0.5;
		IsShowing = false;

		GetRPCManager().AddRPC( "COT_ESP", "RequestFullMapESP", this, SingeplayerExecutionType.Both );

		GetPermissionsManager().RegisterPermission( "ESP.View" );
	}

	void ~JMESPModule()
	{
		Hide();
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "ESP.View" );
	}

	override string GetInputToggle()
	{
		return "UACOTToggleESP";
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/esp_form.layout";
	}

	override string GetTitle()
	{
		return "ESP Tools";
	}
	
	override string GetIconName()
	{
		return "X";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override void OnClientPermissionsUpdated()
	{
		super.OnClientPermissionsUpdated();

		for ( int i = 0; i < m_ViewTypes.Count(); i++ )
		{
			m_ViewTypes[i].HasPermission = GetPermissionsManager().HasPermission( "ESP.View." + m_ViewTypes[i].Permission );
		}

		JMESPForm form;
		if ( Class.CastTo( form, GetForm() ) )
		{
			form.DisableToggleableOptions();
		}
	}

	override void OnInit()
	{
		TTypenameArray espTypes = new TTypenameArray;
		RegisterTypes( espTypes );
		
		for(int i = 0; i < espTypes.Count(); i++)
		{
			if ( espTypes[i].IsInherited( JMESPViewType ) )
			{
				ref JMESPViewType viewType = espTypes[i].Spawn();
				if ( viewType )
				{
					m_ViewTypes.Insert( viewType );
					GetPermissionsManager().RegisterPermission( "ESP.View." + viewType.Permission );
				}
			}
		}
	}

	void RegisterTypes( out TTypenameArray types )
	{
		types.Insert( JMESPViewTypePlayer );
		types.Insert( JMESPViewTypeInfected );
		types.Insert( JMESPViewTypeAnimal );
		types.Insert( JMESPViewTypeCar );
		types.Insert( JMESPViewTypeBaseBuilding );
		types.Insert( JMESPViewTypeBoltActionRifle );
		types.Insert( JMESPViewTypeRifle );
		types.Insert( JMESPViewTypePistol );
		types.Insert( JMESPViewTypeWeapon );
		types.Insert( JMESPViewTypeItemTool );
		types.Insert( JMESPViewTypeItemCrafted );
		types.Insert( JMESPViewTypeItemTent );
		types.Insert( JMESPViewTypeItemMaterial );
		types.Insert( JMESPViewTypeItemAttachment );
		types.Insert( JMESPViewTypeItemFood );
		types.Insert( JMESPViewTypeItemExplosive );
		types.Insert( JMESPViewTypeItemBook );
		types.Insert( JMESPViewTypeItemContainer );
		types.Insert( JMESPViewTypeItemEyewear );
		types.Insert( JMESPViewTypeItemAmmo );
		types.Insert( JMESPViewTypeItem );
	}

	array< ref JMESPViewType > GetViewTypes()
	{
		return m_ViewTypes;
	}

	override void OnMissionStart()
	{
		if ( IsMissionClient() )
		{
			JMESPWidget.espModule = this;

			GetGame().GameScript.Call( this, "ThreadESP", NULL );
		}
	}

	void RequestFullMapESP( CallType type, ref ParamsReadContext ctx, PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "ESP.Manipulation.Delete", senderRPC ) )
			return;

		if ( type == CallType.Server )
		{
			PlayerBase player = GetPlayerObjectByIdentity( senderRPC );
			if ( player )
			{
				GetCommunityOnlineToolsBase().Log( senderRPC, "Entering Full Map ESP" );
				GetGame().ObjectDelete( player );
			}
			
			GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
		}
		
		if ( type == CallType.Client )
		{
			COTPlayerIsRemoved = true;
		}
	}

	void EnableFullMap()
	{
		if ( CurrentActiveCamera == NULL )
			return;

		GetRPCManager().SendRPC( "COT_ESP", "RequestFullMapESP", new Param );
	}

	override void OnMissionFinish()
	{
		for (int j = 0; j < m_ActiveESPObjects.Count(); j++ )
		{
			m_ActiveESPObjects[j].Destroy();
		}

		m_ActiveESPObjects.Clear();
	}

	void CreateNewWidgets()
	{
		m_IsCreatingWidgets = true;

		for ( int i = 0; i < m_ESPToCreate.Count(); ++i )
		{
			m_ESPToCreate[i].Create( this );

			m_ActiveESPObjects.Insert( m_ESPToCreate[i] );
		}

		m_ESPToCreate.Clear();

		m_IsCreatingWidgets = false;
	}

	void DestroyOldWidgets()
	{
		m_IsDestroyingWidgets = true;

		for ( int i = 0; i < m_ESPToDestroy.Count(); ++i )
		{
			m_ESPToDestroy[i].Destroy();

			int remove_index = m_ActiveESPObjects.Find( m_ESPToDestroy[i] );
			if ( remove_index >= 0 )
				m_ActiveESPObjects.Remove( remove_index );
		}

		m_ESPToDestroy.Clear();

		m_IsDestroyingWidgets = false;
	}

	void ThreadESP()
	{
		while ( true )
		{
			if ( ( IsShowing || ToDestroy ) && !m_IsDestroyingWidgets && !m_IsCreatingWidgets )
			{
				array<Object> objects = new array<Object>;
				array<Object> addedObjects = new array<Object>;

				GetGame().GetObjectsAtPosition( GetCurrentPosition(), ESPRadius, objects, NULL );

				bool isUsingFilter = Filter.Length() > 0;

				string filter = Filter + "";
				filter.ToLower();

				int k;

				if ( ToDestroy )
				{
					for ( k = m_ActiveESPObjects.Count() - 1; k >= 0; --k )
					{
						m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );
					}
				} else if ( IsShowing )
				{
					for ( int i = 0; i < objects.Count(); ++i )
					{
						Object obj = objects[i];

						if ( obj == NULL )
							continue;

						string type = obj.GetType();
						type.ToLower();

						if ( !IsMissionOffline() && !obj.HasNetworkID() )
							continue;

						if ( obj.IsRock() )
							continue;

						if ( obj.IsWoodBase() )
							continue;

						if ( obj.IsBush() )
							continue;

						if ( obj.IsTree() )
							continue;

						if ( obj.IsBuilding() && !obj.IsInherited( GardenBase ) )
							continue;

						if ( isUsingFilter && !type.Contains( filter ) )
							continue;

						JMESPMeta meta = m_MappedESPObjects.Get( obj );
						if ( meta != NULL )
						{
							if ( meta.IsValid() )
							{
								addedObjects.Insert( obj );
							}
						} else
						{
							for ( int j = 0; j < m_ViewTypes.Count(); j++ )
							{
								if ( m_ViewTypes[j].IsValid( obj, meta ) )
								{
									m_MappedESPObjects.Insert( obj, meta );

									m_ESPToCreate.Insert( meta );

									j = m_ViewTypes.Count();
								}
							}
						}
					}

					for ( k = m_ActiveESPObjects.Count() - 1; k >= 0; --k )
					{
						if ( m_ActiveESPObjects[k].target == NULL )
						{
							m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );
						} else if ( addedObjects.Find( m_ActiveESPObjects[k].target ) == -1 )
						{
							m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );
						}
					}

					GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( CreateNewWidgets );
				}
			
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DestroyOldWidgets );
				
				if ( ToDestroy )
				{
					ESPIsUpdating = false;
					IsShowing = false;
					ToDestroy = false;
				}

				Sleep( ESPUpdateTime * 1000 );

				if ( !ESPIsUpdating )
					IsShowing = false;
			} else
			{
				if ( ( m_IsDestroyingWidgets || m_IsCreatingWidgets ) && ( IsShowing || ToDestroy ) )
					Sleep( 50 );
				else 
					Sleep( 500 );
			}
		}
	}

	override int GetRPCMin()
	{
		return JMESPModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMESPModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMESPModuleRPC.Log:
			RPC_Log( ctx, sender, target );
			break;
		case JMESPModuleRPC.SetPosition:
			RPC_SetPosition( ctx, sender, target );
			break;
		case JMESPModuleRPC.SetOrientation:
			RPC_SetOrientation( ctx, sender, target );
			break;
		case JMESPModuleRPC.SetHealth:
			RPC_SetHealth( ctx, sender, target );
			break;
		}
	}

	void Log( string log )
	{
		if ( IsMissionOffline() )
		{
			Exec_Log( log, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( log );
			rpc.Send( NULL, JMESPModuleRPC.Log, false, NULL );
		}
	}

	private void Exec_Log( string log, PlayerIdentity ident )
	{
		GetCommunityOnlineToolsBase().Log( ident, "ESP: " + log );
	}

	private void RPC_Log( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string log;
		if ( !ctx.Read( log ) )
			return;

		Exec_Log( log, senderRPC );
	}

	void SetPosition( vector position, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetPosition( position, target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( position );
			rpc.Send( target, JMESPModuleRPC.SetPosition, false, NULL );
		}
	}

	private void Exec_SetPosition( vector position, Object target, PlayerIdentity ident )
	{
		target.SetPosition( position );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " position=" + position );
	}

	private void RPC_SetPosition( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector position;
		if ( !ctx.Read( position ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Object.Set.Position", senderRPC ) )
			return;

		Exec_SetPosition( position, target, senderRPC );
	}

	void SetOrientation( vector orientation, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetOrientation( orientation, target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( orientation );
			rpc.Send( target, JMESPModuleRPC.SetOrientation, false, NULL );
		}
	}

	private void Exec_SetOrientation( vector orientation, Object target, PlayerIdentity ident )
	{
		target.SetOrientation( orientation );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " orientation=" + orientation );
	}

	private void RPC_SetOrientation( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector orientation;
		if ( !ctx.Read( orientation ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Object.Set.Orientation", senderRPC ) )
			return;

		Exec_SetOrientation( orientation, target, senderRPC );
	}

	void SetHealth( float health, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetHealth( health, target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( health );
			rpc.Send( target, JMESPModuleRPC.SetHealth, false, NULL );
		}
	}

	private void Exec_SetHealth( float health, Object target, PlayerIdentity ident )
	{
		target.SetHealth( health );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " health=" + health );
	}

	private void RPC_SetHealth( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float health;
		if ( !ctx.Read( health ) )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.Object.Set.Health", senderRPC ) )
			return;

		Exec_SetHealth( health, target, senderRPC );
	}
}