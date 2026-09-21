class UIActionMapController
{
	protected UIActionMap m_Map;
	protected bool m_PlayerTrackingEnabled;
	protected bool m_ClickToPickEnabled;
	protected vector m_PickedWorldPos;
	protected ref map<string, bool> m_LayerVisibility;
	protected Class m_CallbackInstance;
	protected string m_CallbackFunction;

	void UIActionMapController( UIActionMap actionMap )
	{
		m_Map = actionMap;
		m_PlayerTrackingEnabled = false;
		m_ClickToPickEnabled = true;
		m_PickedWorldPos = "0 0 0";
		m_LayerVisibility = new map<string, bool>();

		if ( m_Map )
		{
			m_Map.SetCallback( this, "OnMapClickInternal" );
		}
	}

	void ~UIActionMapController()
	{
		delete m_LayerVisibility;
	}

	UIActionMap GetMap()
	{
		return m_Map;
	}

	vector GetPickedWorldPosition()
	{
		return m_PickedWorldPos;
	}

	bool IsClickToPickEnabled()
	{
		return m_ClickToPickEnabled;
	}

	bool IsLayerVisible( string layer )
	{
		bool visible = true;
		if ( m_LayerVisibility.Find( layer, visible ) )
			return visible;

		return true;
	}

	bool IsPlayerTrackingEnabled()
	{
		return m_PlayerTrackingEnabled;
	}

	void SetCallback( Class instance, string functionName )
	{
		m_CallbackInstance = instance;
		m_CallbackFunction = functionName;
	}

	void SetClickToPickPosition( bool enable )
	{
		m_ClickToPickEnabled = enable;
	}

	void SetLayerVisible( string layer, bool visible )
	{
		m_LayerVisibility.Set( layer, visible );

		if ( !visible && m_Map )
		{
			m_Map.ClearLayer( layer );
		}
	}

	void EnablePlayerTracking( bool enable )
	{
		m_PlayerTrackingEnabled = enable;
		if ( !m_PlayerTrackingEnabled && m_Map )
		{
			m_Map.ClearLayer( "players" );
		}
	}

	void AddWaypoint( string id, vector pos, string label, int color = 0xFFFFFFFF, string icon = "", string layer = "waypoints" )
	{
		if ( !m_Map || !IsLayerVisible( layer ) )
			return;

		m_Map.AddMarker( id, pos, label, color, icon, layer );
	}

	void RemoveWaypoint( string id )
	{
		if ( m_Map )
			m_Map.RemoveMarker( id );
	}

	void ClearLayer( string layer )
	{
		if ( m_Map )
			m_Map.ClearLayer( layer );
	}

	void CenterOn( vector pos )
	{
		if ( m_Map )
			m_Map.CenterOn( pos );
	}

	void UpdatePlayerTracking()
	{
		if ( !m_Map || !m_PlayerTrackingEnabled || !IsLayerVisible( "players" ) )
			return;

		if ( !GetPermissionsManager() )
			return;

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();
		if ( !players )
			return;

		foreach ( JMPlayerInstance p : players )
		{
			if ( !p )
				continue;

			PlayerBase playerObj = p.PlayerObject;
			if ( !playerObj )
				continue;

			vector playerPos = playerObj.GetPosition();
			string markerId = "player_" + p.GetGUID();
			int pColor = JMTheme.ACCENT;

			m_Map.AddMarker( markerId, playerPos, p.GetName(), pColor, "set:dayz_gui image:icon_player", "players" );
		}
	}

	void Tick()
	{
		if ( m_Map )
			m_Map.TickMarkers();

		if ( m_PlayerTrackingEnabled )
			UpdatePlayerTracking();
	}

	protected void OnMapClickInternal( UIEvent eid = 0, UIActionBase action = null )
	{
		if ( !m_Map )
			return;

		vector rawPos = m_Map.GetLastClickWorldPos();

		if ( m_ClickToPickEnabled && g_Game )
		{
			float surfaceY = g_Game.SurfaceY( rawPos[0], rawPos[2] );
			rawPos[1] = surfaceY;
			m_PickedWorldPos = rawPos;
		}

		NotifyCallback();
	}

	protected void NotifyCallback()
	{
		if ( m_CallbackInstance && m_CallbackFunction != "" )
		{
			g_Game.GameScript.CallFunction( m_CallbackInstance, m_CallbackFunction, null, this );
		}
	}
}
