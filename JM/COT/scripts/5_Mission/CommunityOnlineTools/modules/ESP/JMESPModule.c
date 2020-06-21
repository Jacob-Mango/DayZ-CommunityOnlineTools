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

	void CreateNewWidgets()
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

	void DestroyOldWidgets()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPModule::DestroyOldWidgets() void;" );
		#endif

		m_IsDestroyingWidgets = true;

		for ( int i = 0; i < m_ESPToDestroy.Count(); ++i )
		{
			#ifdef JM_COT_ESP_DEBUG
			Print( "  Removing: " + m_ESPToDestroy[i] );
			#endif

			if ( m_ESPToDestroy[i] )
			{
				int remove_index = m_ActiveESPObjects.Find( m_ESPToDestroy[i] );
				#ifdef JM_COT_ESP_DEBUG
				Print( "  remove_index: " + remove_index );
				#endif
				
				if ( remove_index >= 0 )
					m_ActiveESPObjects.Remove( remove_index );

				#ifdef JM_COT_ESP_DEBUG
				Print( "  Removed." );
				#endif

				m_ESPToDestroy[i].Destroy();
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

	private void ChunkGetObjects( out array<Object> objects, out int totalTimeTaken )
	{
		vector centerPosition = GetCurrentPosition();

		float maxRadiusChunk = 100;

		int numIterations = Math.Ceil( ESPRadius / maxRadiusChunk );
		Print( centerPosition );
		Print( ESPRadius );
		Print( maxRadiusChunk );
		Print( numIterations );

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

		totalTimeTaken += 10;
		Sleep( 10 );

		for ( int currIter = 2; currIter <= numIterations; ++currIter )
		{
			Print( currIter );
			int numSubChunks = currIter * currIter;
			Print( numSubChunks );
			for ( int chunkIdx = 1; chunkIdx <= numSubChunks; ++chunkIdx )
			{
				Print( chunkIdx );
				subObjects.Clear();

				vector chunkPos = GetChunkCenterPosition( centerPosition, maxRadiusChunk, currIter, chunkIdx, numSubChunks );

				Print( chunkPos );
				GetGame().GetObjectsAtPosition( chunkPos, maxRadiusChunk, subObjects, NULL );

				for ( i = 0; i < subObjects.Count(); i++ )
				{
					if ( objects.Find( subObjects[i] ) < 0 )
					{
						objects.Insert( subObjects[i] );
					}
				}
				subObjects.Clear();

				totalTimeTaken += 10;
				Sleep( 10 );
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

	void ThreadESP()
	{
		while ( true )
		{
			int totalTimeTaken = 0;

			if ( ( IsShowing || ToDestroy ) && !m_IsDestroyingWidgets && !m_IsCreatingWidgets )
			{
				array<Object> objects = new array<Object>;
				array<Object> addedObjects = new array<Object>;

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

					totalTimeTaken += 1;
					Sleep( 1 );

					for ( k = m_MappedESPObjects.Count() - 1; k >= 0; --k )
					{
						if ( m_MappedESPObjects.GetKey( i ) == NULL )
						{
							m_MappedESPObjects.RemoveElement( i );
						}
					}

					totalTimeTaken += 1;
					Sleep( 1 );

					#ifdef JM_COT_ESP_DEBUG
					Print( "+JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif

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

					#ifdef JM_COT_ESP_DEBUG
					Print( "-JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif

					GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( CreateNewWidgets );
				}
			
				GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DestroyOldWidgets );
				
				if ( ToDestroy )
				{
					ESPIsUpdating = false;
					IsShowing = false;
					ToDestroy = false;
				}

				int sleepTime = Math.Max( 0, ( ESPUpdateTime * 1000 ) - totalTimeTaken );
				Sleep( sleepTime );

				if ( !ESPIsUpdating )
				{
					IsShowing = false;
				}
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