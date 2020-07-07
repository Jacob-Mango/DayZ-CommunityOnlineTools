enum JMESPModuleRPC
{
	INVALID = 10300,
	Log,
	FullMap,
	SetPosition,
	SetOrientation,
	SetHealth,
	DeleteObject,
	BaseBuilding_Build,
	BaseBuilding_Dismantle,
	BaseBuilding_Repair,
	COUNT
};

enum JMESPState
{
	UNKNOWN = 0,
	Update,
	View,
	Remove
};

class JMESPModule: JMRenderableModuleBase
{
	private ref array< Object > m_SelectedObjects;
	
	private ref array< ref JMESPMeta > m_ActiveESPObjects;

	private ref array< ref JMESPMeta > m_ESPToCreate;
	private ref array< JMESPMeta > m_ESPToDestroy;

	private ref map< Object, JMESPMeta > m_MappedESPObjects;

	private ref array< ref JMESPViewType > m_ViewTypes;

	private bool m_IsCreatingWidgets;
	private bool m_IsDestroyingWidgets;

	string Filter;

	float ESPRadius;
	int ESPUpdateTime;

	private JMESPState m_CurrentState = JMESPState.Remove;
	private bool m_StateChanged = false;

	void JMESPModule()
	{
		ESPRadius = 200;

		ESPUpdateTime = 1;

		GetRPCManager().AddRPC( "COT_ESP", "RequestFullMapESP", this, SingeplayerExecutionType.Both );

		GetPermissionsManager().RegisterPermission( "ESP.View" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.SetPosition" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetOrientation" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetHealth" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Delete" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Build" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Build.MaterialsNotRequired" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Dismantle" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Repair" );
	}

	void ~JMESPModule()
	{
		delete m_SelectedObjects;
		delete m_ActiveESPObjects;
		delete m_ESPToCreate;
		delete m_ESPToDestroy;
		delete m_MappedESPObjects;
		delete m_ViewTypes;

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
		return "#STR_COT_ESP_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "X";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "ESP Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Log" );
		types.Insert( "Position" );
		types.Insert( "Orientation" );
		types.Insert( "Health" );
		types.Insert( "Delete" );
		types.Insert( "BB_Build" );
		types.Insert( "BB_Dismantle" );
		types.Insert( "BB_Repair" );
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
		m_SelectedObjects = new array< Object >;

		m_ActiveESPObjects = new array< ref JMESPMeta >;

		m_ESPToCreate = new array< ref JMESPMeta >;
		m_ESPToDestroy = new array< JMESPMeta >;

		m_MappedESPObjects = new map< Object, JMESPMeta >;

		m_ViewTypes = new array< ref JMESPViewType >;

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
		types.Insert( JMESPViewTypeBoltActionRifle );
		types.Insert( JMESPViewTypeBoltRifle );
		types.Insert( JMESPViewTypeRifle );
		types.Insert( JMESPViewTypePistol );
		types.Insert( JMESPViewTypeUnknown );
		types.Insert( JMESPViewTypeTent );
		types.Insert( JMESPViewTypeBaseBuilding );
		types.Insert( JMESPViewTypeFood );
		types.Insert( JMESPViewTypeExplosive );
		types.Insert( JMESPViewTypeBook );
		types.Insert( JMESPViewTypeContainer );
		types.Insert( JMESPViewTypeTransmitter );
		types.Insert( JMESPViewTypeClothing );
		types.Insert( JMESPViewTypeMagazine );
		types.Insert( JMESPViewTypeAmmo );
	}

	array< ref JMESPViewType > GetViewTypes()
	{
		return m_ViewTypes;
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		if ( IsMissionClient() )
		{
			JMESPWidgetHandler.espModule = this;

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

	private void CreateNewWidgets()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPModule::CreateNewWidgets() void;" );
		#endif

		m_IsCreatingWidgets = true;

		for ( int i = 0; i < m_ESPToCreate.Count(); ++i )
		{
			m_ESPToCreate[i].Create( this );

			m_ActiveESPObjects.Insert( m_ESPToCreate[i] );
		}

		m_ESPToCreate.Clear();

		m_IsCreatingWidgets = false;

		#ifdef JM_COT_ESP_DEBUG
		Print( "-JMESPModule::CreateNewWidgets() void;" );
		#endif
	}

	private void DestroyOldWidgets()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPModule::DestroyOldWidgets() void;" );
		#endif

		m_IsDestroyingWidgets = true;

		for ( int i = 0; i < m_ESPToDestroy.Count(); ++i )
		{
			JMESPMeta meta = m_ESPToDestroy[i];

			#ifdef JM_COT_ESP_DEBUG
			Print( "  Removing: " + meta );
			#endif

			if ( meta )
			{
				int remove_index = m_ActiveESPObjects.Find( meta );
				#ifdef JM_COT_ESP_DEBUG
				Print( "  remove_index: " + remove_index );
				#endif
				
				if ( remove_index >= 0 )
					m_ActiveESPObjects.Remove( remove_index );

				#ifdef JM_COT_ESP_DEBUG
				Print( "  Removed." );
				#endif

				delete meta;
			}
		}

		#ifdef JM_COT_ESP_DEBUG
		Print( "  Clearing m_ESPToDestroy" );
		#endif

		m_ESPToDestroy.Clear();

		m_IsDestroyingWidgets = false;

		#ifdef JM_COT_ESP_DEBUG
		Print( "-JMESPModule::DestroyOldWidgets() void;" );
		#endif
	}

	void _Sleep( int time, out int totalTimeTaken )
	{
		Sleep( time );
		totalTimeTaken += time;
	}

	void _Sleep( int time, out int totalTimeTaken, inout int sleepIdx )
	{
		sleepIdx += 1;
		if ( sleepIdx % 5 == 0 )
		{
			Sleep( time );
			totalTimeTaken += time;
		}
	}

	private void ChunkGetObjects( out array<Object> objects, out int totalTimeTaken )
	{
		vector centerPosition = GetCurrentPosition();

		float maxRadiusChunk = 100;

		int numIterations = Math.Ceil( ESPRadius / maxRadiusChunk );

		array<Object> subObjects = new array<Object>;
		GetGame().GetObjectsAtPosition( centerPosition, maxRadiusChunk, subObjects, NULL );

		for ( int i = 0; i < subObjects.Count(); i++ )
		{
			if ( objects.Find( subObjects[i] ) < 0 )
			{
				objects.Insert( subObjects[i] );
			}
		}

		subObjects.Clear();

		int sleepIdx = 0;

		_Sleep( 1, totalTimeTaken, sleepIdx );

		for ( int currIter = 2; currIter <= numIterations; ++currIter )
		{
			int numSubChunks = currIter * currIter;
			for ( int chunkIdx = 1; chunkIdx <= numSubChunks; ++chunkIdx )
			{
				subObjects.Clear();

				vector chunkPos = GetChunkCenterPosition( centerPosition, maxRadiusChunk, currIter, chunkIdx, numSubChunks );

				GetGame().GetObjectsAtPosition( chunkPos, maxRadiusChunk, subObjects, NULL );

				for ( i = 0; i < subObjects.Count(); i++ )
				{
					if ( objects.Find( subObjects[i] ) < 0 )
					{
						objects.Insert( subObjects[i] );
					}
				}
				subObjects.Clear();

				_Sleep( 1, totalTimeTaken, sleepIdx );
			}
		}
	}

	vector GetChunkCenterPosition( vector center, float radiusSize, int chnkIdx, int index, int count )
	{
		float angle = Math.PI - ( Math.PI2 * index / count );

		float distance = chnkIdx * radiusSize;

		float x = distance * Math.Cos( angle );
		float z = distance * Math.Sin( angle );

		return center + Vector( x, 0, z );
	}

	JMESPState GetState()
	{
		return m_CurrentState;
	}

	bool IsStateChangeProcessing()
	{
		return m_StateChanged;
	}

	void UpdateState( JMESPState newState )
	{
		m_CurrentState = newState;
		m_StateChanged = true;
	}

	void ThreadESP()
	{
		while ( true )
		{
			int totalTimeTaken = 0;
			bool didRun = false;

			if ( m_StateChanged && !m_IsDestroyingWidgets && !m_IsCreatingWidgets )
			{
				m_StateChanged = false;

				didRun = true;

				array<Object> objects = new array<Object>;
				array<Object> addedObjects = new array<Object>;

				bool isUsingFilter = Filter.Length() > 0;

				string filter = Filter + "";
				filter.ToLower();

				int k;

				if ( m_CurrentState == JMESPState.Remove )
				{
					for ( k = m_ActiveESPObjects.Count() - 1; k >= 0; --k )
					{
						m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );
					}
				} else if ( m_CurrentState == JMESPState.View || m_CurrentState == JMESPState.Update )
				{
					ChunkGetObjects( objects, totalTimeTaken );

					for ( int i = 0; i < objects.Count(); ++i )
					{
						Object obj = objects[i];

						if ( obj == NULL )
							continue;

						string type = obj.GetType();
						type.ToLower();

						if ( !IsMissionOffline() && !obj.HasNetworkID() )
							continue;

						if ( obj.GetType() == "" )
							continue;

						if ( obj.GetType() == "#particlesourceenf" )
							continue;

						if ( obj.IsInherited( Particle ) )
							continue;

						if ( obj.IsInherited( Camera ) )
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
							#ifdef JM_COT_ESP_DEBUG
							bool metaIsValid = meta.IsValid();
							Print( "-" + meta.ClassName() + "::IsValid() = " + metaIsValid );
							if ( metaIsValid )
							#else
							if ( meta.IsValid() )
							#endif
							{
								addedObjects.Insert( obj );
							}
						} else
						{
							array< JMESPViewType > validViewTypes = new array< JMESPViewType >;
							for ( int j = 0; j < m_ViewTypes.Count(); j++ )
							{
								if ( m_ViewTypes[j].HasPermission && m_ViewTypes[j].View )
								{
									validViewTypes.Insert( m_ViewTypes[j] );
								}
							}

							for ( j = 0; j < validViewTypes.Count(); j++ )
							{
								#ifdef JM_COT_ESP_DEBUG
								bool viewTypeIsValid = validViewTypes[j].IsValid( obj, meta );
								Print( "-" + validViewTypes[j].ClassName() + "::IsValid( obj = " + Object.GetDebugName( obj ) + ", out = " + meta + " ) = " + viewTypeIsValid );
								if ( viewTypeIsValid )
								#else
								if ( validViewTypes[j].IsValid( obj, meta ) )
								#endif
								{
									m_MappedESPObjects.Set( obj, meta );

									m_ESPToCreate.Insert( meta );

									j = validViewTypes.Count();
								}
							}
						}
					}

					_Sleep( 1, totalTimeTaken );

					#ifdef JM_COT_ESP_DEBUG
					Print( "+JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif

					for ( k = m_ActiveESPObjects.Count() - 1; k >= 0; --k )
					{
						Object aTgt = m_ActiveESPObjects[k].target;

						if ( aTgt == NULL )
						{
							m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );
						} else if ( addedObjects.Find( aTgt ) == -1 )
						{
							m_ESPToDestroy.Insert( m_ActiveESPObjects[k] );

							m_MappedESPObjects.Remove( aTgt );
						}
					}

					m_MappedESPObjects.Remove( NULL );

					#ifdef JM_COT_ESP_DEBUG
					Print( "-JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif

					GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( CreateNewWidgets );
				}
			
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DestroyOldWidgets );
				
				if ( m_CurrentState == JMESPState.Update )
				{
					m_StateChanged = true;
				}
			}

			if ( didRun && m_StateChanged && m_CurrentState == JMESPState.Update )
			{				
				Sleep( Math.Max( 0, ( ESPUpdateTime * 1000 ) - totalTimeTaken ) );
			} else
			{
				Sleep( 50 );
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
		case JMESPModuleRPC.DeleteObject:
			RPC_DeleteObject( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_Build:
			RPC_BaseBuilding_Build( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_Dismantle:
			RPC_BaseBuilding_Dismantle( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_Repair:
			RPC_BaseBuilding_Repair( ctx, sender, target );
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

	private void Exec_Log( string log, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		GetCommunityOnlineToolsBase().Log( ident, "ESP: " + log );
		SendWebhook( "Log", instance, "Logging ESP action: " + log );
	}

	private void RPC_Log( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string log;
		if ( !ctx.Read( log ) )
			return;

		Exec_Log( log, senderRPC, GetPermissionsManager().GetPlayer( senderRPC.GetId() ) );
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

	private void Exec_SetPosition( vector position, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		target.SetPosition( position );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=position value=" + position );
		SendWebhook( "Position", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") position to " + position.ToString() );
	}

	private void RPC_SetPosition( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector position;
		if ( !ctx.Read( position ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetPosition", senderRPC, instance ) )
			return;

		Exec_SetPosition( position, target, senderRPC, instance );
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

	private void Exec_SetOrientation( vector orientation, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		target.SetOrientation( orientation );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=orientation value=" + orientation );
		SendWebhook( "Orientation", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") orientation to " + orientation.ToString() );
	}

	private void RPC_SetOrientation( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		vector orientation;
		if ( !ctx.Read( orientation ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetOrientation", senderRPC ) )
			return;

		Exec_SetOrientation( orientation, target, senderRPC, instance );
	}

	void SetHealth( float health, string zone, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetHealth( health, zone, target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( health );
			rpc.Send( target, JMESPModuleRPC.SetHealth, false, NULL );
		}
	}

	private void Exec_SetHealth( float health, string zone, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		target.SetHealth( health );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=health value=" + health );
		SendWebhook( "Health", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") health to " + health );
	}

	private void RPC_SetHealth( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float health;
		if ( !ctx.Read( health ) )
			return;

		string zone;
		if ( !ctx.Read( zone ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetHealth", senderRPC, instance ) )
			return;

		Exec_SetHealth( health, zone, target, senderRPC, instance );
	}

	void DeleteObject( Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_DeleteObject( target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( target, JMESPModuleRPC.DeleteObject, false, NULL );
		}
	}

	private void Exec_DeleteObject( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		string obtype;
		GetGame().ObjectGetType( target, obtype );

		vector transform[4];
		target.GetTransform( transform );

		GetGame().ObjectDelete( target );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + obtype + " position=" + transform[3].ToString() + " action=delete" );
		SendWebhook( "Delete", instance, "Deleted " + obtype + " at " + transform[3].ToString() );
	}

	private void RPC_DeleteObject( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Delete", senderRPC, instance ) )
			return;

		Exec_DeleteObject( target, senderRPC, instance );
	}

	void BaseBuilding_Build( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Build( target, part, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( part );
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Build, false, NULL );
		}
	}

	private void Exec_BaseBuilding_Build( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
        bool requireMaterials = true;
        if ( !IsMissionOffline() )
            requireMaterials = !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Build.MaterialsNotRequired", ident, instance );
		
        target.GetConstruction().COT_BuildPart( part_name, requireMaterials );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=built part=" + part_name + " required_materials=" + requireMaterials );
		SendWebhook( "BB_Build", instance, "Built the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Build( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Build", senderRPC, instance ) )
			return;

        BaseBuildingBase bb;
        if ( Class.CastTo( bb, target ) )
		    Exec_BaseBuilding_Build( bb, part_name, senderRPC, instance );
	}

	void BaseBuilding_Dismantle( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Dismantle( target, part, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
            rpc.Write( part );
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Dismantle, false, NULL );
		}
	}

	private void Exec_BaseBuilding_Dismantle( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

        target.GetConstruction().COT_DismantlePart( part_name, player );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=dismantle part=" + part_name  );
		SendWebhook( "BB_Dismantle", instance, "Dismantled the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Dismantle( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Dismantle", senderRPC, instance ) )
			return;

        BaseBuildingBase bb;
        if ( Class.CastTo( bb, target ) )
		    Exec_BaseBuilding_Dismantle( bb, part_name, senderRPC, instance );
	}

	void BaseBuilding_Repair( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Repair( target, part, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
            rpc.Write( part );
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Repair, false, NULL );
		}
	}

	private void Exec_BaseBuilding_Repair( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
        target.GetConstruction().COT_RepairPart( part_name );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=repair part=" + part_name  );
		SendWebhook( "BB_Repair", instance, "Repaired the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Repair( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Repair", senderRPC, instance ) )
			return;

        BaseBuildingBase bb;
        if ( Class.CastTo( bb, target ) )
		    Exec_BaseBuilding_Repair( bb, part_name, senderRPC, instance );
	}

	private void OnAddObject( Object obj )
	{
		if ( m_SelectedObjects.Find( obj ) != -1 )
			return;

		m_SelectedObjects.Insert( obj );
	}

	private void OnRemoveObject( Object obj )
	{
		int index = m_SelectedObjects.Find( obj );
		if ( index == -1 )
			return;

		m_SelectedObjects.Remove( index );
	}
	
	void MakeItemSet( string name )
	{
		COTCreateLocalAdminNotification( new StringLocaliser( "Creating Item Set: %1", name ) );
	}

	private void RPC_MakeItemSet( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	}

	private void Exec_MakeItemSet( array< Object > objects, string name, PlayerIdentity ident )
	{
	}

	void DuplicateAll()
	{
	}

	private void RPC_DuplicateAll( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	}

	private void Exec_DuplicateAll( array< Object > objects, PlayerIdentity ident )
	{
	}

	void DeleteAll()
	{
	}

	private void RPC_DeleteAll( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	}

	private void Exec_DeleteAll( array< Object > objects, PlayerIdentity ident )
	{
	}

	void MoveToCursorRelative()
	{
	}

	private void RPC_MoveToCursorRelative( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	}

	private void Exec_MoveToCursorRelative( array< Object > objects, PlayerIdentity ident )
	{
	}

	void MoveToCursorAbsolute()
	{
	}

	private void RPC_MoveToCursorAbsolute( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
	}

	private void Exec_MoveToCursorAbsolute( array< Object > objects, PlayerIdentity ident )
	{
	}
}