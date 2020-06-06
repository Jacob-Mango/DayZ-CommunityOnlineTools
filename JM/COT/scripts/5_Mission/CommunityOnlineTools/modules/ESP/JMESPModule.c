enum JMESPModuleRPC
{
	INVALID = 10300,
	Log,
	FullMap,
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

		GetRPCManager().AddRPC( "COT_ESP", "ESPLog", this, SingeplayerExecutionType.Server );
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
			m_ESPToCreate[i].Create();

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
								meta = new JMESPMeta;
								if ( m_ViewTypes[j].IsValid( obj, meta ) )
								{
									m_MappedESPObjects.Insert( obj, meta );

									m_ESPToCreate.Insert( meta );

									j = m_ViewTypes.Count();
								} else
								{
									delete meta;
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
				if ( m_IsDestroyingWidgets || m_IsCreatingWidgets )
					Sleep( 100 );
				else 
					Sleep( 500 );
			}
		}
	}

	void ESPLog( CallType type, ref ParamsReadContext ctx, PlayerIdentity senderRPC, ref Object target )
	{
		ref Param1< string > data;
		if ( !ctx.Read( data ) ) return;
		
		if ( type == CallType.Server )
		{
			GetCommunityOnlineToolsBase().Log( senderRPC, "ESP " + data.param1 );
		}
	}
}