class JMESPMeta: COT_WidgetHolder
{
#ifdef DIAG
	static int s_JMESPMetaCount;
#endif

	static ref CF_DoublyLinkedNodes_WeakRef<JMESPMeta> s_JM_All = new CF_DoublyLinkedNodes_WeakRef<JMESPMeta>();

	ref CF_DoublyLinkedNode_WeakRef<JMESPMeta> s_JM_Node;

	string name;
	JMESPViewType type;
	int colour;
	JMPlayerInstance player;
	Object target;

	int networkLow;
	int networkHigh;

	JMESPModule module;
	JMESPWidgetHandler widgetHandler;
	Widget widgetRoot;
	Widget viewTypeActions;

	UIActionEditableText m_Action_PositionX;
	UIActionEditableText m_Action_PositionY;
	UIActionEditableText m_Action_PositionZ;
	UIActionButton m_Action_Position;
	UIActionButton m_Action_GetPosition;
	UIActionButton m_Action_PastePosition;
	UIActionButton m_Action_RefreshPosition;
	UIActionCheckbox m_Action_AutoRefreshPosition;
	
	UIActionEditableText m_Action_OrientationX;
	UIActionEditableText m_Action_OrientationY;
	UIActionEditableText m_Action_OrientationZ;
	UIActionButton m_Action_Orientation;
	UIActionButton m_Action_GetOrientation;
	UIActionButton m_Action_PasteOrientation;
	UIActionButton m_Action_RefreshOrientation;
	UIActionCheckbox m_Action_AutoRefreshOrientation;

	UIActionEditableText m_Action_Health;

	UIActionButton m_Action_Delete;
	UIActionButton m_HealButton;

	bool m_ActionsInitialized;

	void JMESPMeta()
	{
		s_JM_Node = s_JM_All.Add(this);

	#ifdef DIAG
		s_JMESPMetaCount++;
	#endif
	}

	void ~JMESPMeta()
	{
		if (!GetGame())
			return;

		#ifdef JM_COT_ESP_DEBUG
		auto trace = CF_Trace_0(this);
		Print( "  m_Action_Position = " + m_Action_Position );
		Print( "  m_Action_GetPosition = " + m_Action_GetPosition );
		Print( "  m_Action_Orientation = " + m_Action_Orientation );
		Print( "  m_Action_GetOrientation = " + m_Action_GetOrientation );
		Print( "  m_Action_Health = " + m_Action_Health );
		Print( "  widgetHandler = " + widgetHandler );
		#endif

		DestroyWidget(widgetRoot);

		if (s_JM_All)
			s_JM_All.Remove(s_JM_Node);

	#ifdef DIAG
		s_JMESPMetaCount--;
		if (s_JMESPMetaCount <= 0)
			CF_Log.Info("JMESPMeta count: " + s_JMESPMetaCount);
	#endif
	}

	void Create( JMESPModule mod )
	{
		module = mod;

		if ( widgetRoot )
			return;

		if ( !Class.CastTo( widgetRoot, GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout", JMStatics.ESP_CONTAINER ) ) )
			return;

		widgetRoot.GetScript( widgetHandler );
		if ( !widgetHandler )
			return;

		widgetHandler.SetInfo( this, viewTypeActions );
	}

	string GetName()
	{
		string displayName = target.GetDisplayName();

		if (displayName == "")
			displayName = GetType();

		return displayName;
	}

	string GetType()
	{
		return GetObjectType(target);
	}

	static string GetObjectType(Object obj)
	{
		string objType = obj.GetType();

		if (objType == "")
		{
			string debugName = obj.GetDebugName();
			int i = debugName.IndexOf(":");

			if (i > -1)
				objType = debugName.Substring(i + 1, debugName.Length() - i - 1).Trim();
			else if (obj.IsScenery())
				objType = "TERRAIN";
			else
				objType = debugName;
		}

		return objType;
	}

	void InitActions()
	{
		m_ActionsInitialized = true;

		CreateActions( viewTypeActions );

		UpdateActions();
	}

	bool IsValid()
	{
		#ifdef JM_COT_ESP_DEBUG
		Print( "+JMESPMeta::IsValid() bool;" );
		Print( "  target = " + Object.GetDebugName( target ) );
		#endif

		if ( target == NULL )
			return false;

		if ( !(type.HasPermission && type.View) )
			return false;

		JMESPMeta meta = this;
		return type.IsValid( target, meta );
	}

	void CreateActions( Widget parent )
	{
		Widget positionActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget positionActionsVec = UIActionManager.CreateGridSpacer( positionActions, 1, 3 );

		m_Action_PositionX = UIActionManager.CreateEditableText( positionActionsVec, "X:", this, "Action_SetPosition" );
		m_Action_PositionY = UIActionManager.CreateEditableText( positionActionsVec, "Y:", this, "Action_SetPosition" );
		m_Action_PositionZ = UIActionManager.CreateEditableText( positionActionsVec, "Z:", this, "Action_SetPosition" );

		m_Action_PositionX.SetOnlyNumbers( true );
		m_Action_PositionY.SetOnlyNumbers( true );
		m_Action_PositionZ.SetOnlyNumbers( true );
		
		Widget positionActionsButtons = UIActionManager.CreateWrapSpacer( positionActions );

		if (networkLow || networkHigh)
			m_Action_Position = UIActionManager.CreateButton( positionActionsButtons, "Set", this, "Action_SetPosition", 0.25 );

		m_Action_GetPosition = UIActionManager.CreateButton( positionActionsButtons, "C", this, "Action_GetPosition", 0.12 );

		if (networkLow || networkHigh)
		{
			m_Action_PastePosition = UIActionManager.CreateButton( positionActionsButtons, "P", this, "Action_PastePosition", 0.12 );
			m_Action_RefreshPosition = UIActionManager.CreateButton( positionActionsButtons, "Refresh", this, "Action_RefreshPosition", 0.35 );
			m_Action_AutoRefreshPosition = UIActionManager.CreateCheckbox( positionActionsButtons, "", this, "Click_AutoRefreshPosition", false, 0.11 );
		}

		UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

		Widget orientationActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );
		Widget orientationActionsVec = UIActionManager.CreateGridSpacer( orientationActions, 1, 3 );

		m_Action_OrientationX = UIActionManager.CreateEditableText( orientationActionsVec, "X:", this, "Action_SetOrientation" );
		m_Action_OrientationY = UIActionManager.CreateEditableText( orientationActionsVec, "Y:", this, "Action_SetOrientation" );
		m_Action_OrientationZ = UIActionManager.CreateEditableText( orientationActionsVec, "Z:", this, "Action_SetOrientation" );

		m_Action_OrientationX.SetOnlyNumbers( true );
		m_Action_OrientationY.SetOnlyNumbers( true );
		m_Action_OrientationZ.SetOnlyNumbers( true );
		
		Widget orientationActionsButtons = UIActionManager.CreateWrapSpacer( orientationActions );

		if (networkLow || networkHigh)
			m_Action_Orientation = UIActionManager.CreateButton( orientationActionsButtons, "Set", this, "Action_SetOrientation", 0.25 );

		m_Action_GetOrientation = UIActionManager.CreateButton( orientationActionsButtons, "C", this, "Action_GetOrientation", 0.12 );

		if (networkLow || networkHigh)
		{
			m_Action_PasteOrientation = UIActionManager.CreateButton( orientationActionsButtons, "P", this, "Action_PasteOrientation", 0.12 );
			m_Action_RefreshOrientation = UIActionManager.CreateButton( orientationActionsButtons, "Refresh", this, "Action_RefreshOrientation", 0.35 );
			m_Action_AutoRefreshOrientation = UIActionManager.CreateCheckbox( orientationActionsButtons, "", this, "Click_AutoRefreshOrientation", false, 0.11 );
		}

		if ( (networkLow || networkHigh) && MiscGameplayFunctions.GetTypeMaxGlobalHealth(target.GetType()) > 0 )
		{
			UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

			m_Action_Health = UIActionManager.CreateEditableText( parent, "Health: ", this, "Action_SetHealth", "", "Set" );
			m_Action_Health.SetOnlyNumbers( true );

			if ( !target.IsInherited(Man) && !target.IsInherited(DayZCreature) )
				m_HealButton  = UIActionManager.CreateButton( parent, "Repair",  this, "Action_Heal" );
			else
				m_HealButton  = UIActionManager.CreateButton( parent, "Heal",  this, "Action_Heal" );
		}

		if ( (networkLow || networkHigh) && CanDelete() )
		{
			UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

			m_Action_Delete = UIActionManager.CreateButton( parent, "Delete Object", this, "Action_Delete" );
		}

		//! Basic object type properties that are not covered by view types
		TStringArray basicProperties = {};

		if (target.IsBuilding())
			basicProperties.Insert("Building");
		if (target.IsBush())
			basicProperties.Insert("Bush");
		if (target.IsCorpse())
			basicProperties.Insert("Corpse");
		if (target.IsElectricAppliance())
			basicProperties.Insert("Electric Appliance");
		if (target.IsFireplace())
			basicProperties.Insert("Fireplace");
		if (target.IsFruit())
			basicProperties.Insert("Fruit");
		if (target.IsFuelStation())
			basicProperties.Insert("Fuel Station");
		if (target.IsHologram())
			basicProperties.Insert("Hologram");
		if (target.IsMeat())
			basicProperties.Insert("Meat");
		if (target.IsInherited(ToolBase))
			basicProperties.Insert("Tool");
		if (target.IsMushroom())
			basicProperties.Insert("Mushroom");
		if (target.IsParticle())
			basicProperties.Insert("Particle");
		if (target.IsPeltBase())
			basicProperties.Insert("Pelt");
		if (target.IsPlainObject())
			basicProperties.Insert("Plain Object");
		if (target.IsRock())
			basicProperties.Insert("Rock");
		if (target.IsScenery())
			basicProperties.Insert("Scenery");
		if (target.IsScriptedLight())
			basicProperties.Insert("Scripted Light");
		if (target.IsStaticTransmitter())
			basicProperties.Insert("Static Transmitter");
		if (target.IsTree())
			basicProperties.Insert("Tree");
		if (target.IsWell())
			basicProperties.Insert("Well");
		if (target.IsWoodBase())
			basicProperties.Insert("Wood");

		if (basicProperties.Count())
		{
			UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

			//Widget propertyContainer = UIActionManager.CreateWrapSpacer(parent);

			string basicPropertiesText;

			foreach (int i, string property: basicProperties)
			{
				//UIActionManager.CreateText(propertyContainer, "", property, null, "", UIActionHAlign.LEFT, UIActionHAlign.CENTER, UIActionHAlign.LEFT);
				if (i)
					basicPropertiesText += ", ";
				basicPropertiesText += property;
			}

			UIActionManager.CreateText(parent, "Is:", basicPropertiesText);
		}

		if (networkLow || networkHigh)
		{
			UIActionManager.CreatePanel( parent, 0xFF000000, 1 );

			UIActionManager.CreateText(parent, "ID:", networkHigh.ToString() + " " + networkLow.ToString());

		}
	}

	bool CanDelete()
	{
		return true;
	}

	private string FloatToString( float num )
	{
		string ret = num.ToString();
		if ( ret == "" )
			ret = "0";

		return ret;
	}

	void UpdateActions()
	{
		if ( !viewTypeActions || !widgetRoot || !target )
			return;

		RefreshPosition();
		RefreshOrientation();

		if ( m_Action_Health && !m_Action_Health.IsFocused() ) m_Action_Health.SetText( "-1" );
	}

	void Update()
	{
		if ( !target || !m_ActionsInitialized )
			return;

		if (m_Action_AutoRefreshPosition.IsChecked())
			RefreshPosition();

		if (m_Action_AutoRefreshOrientation.IsChecked())
			RefreshOrientation();
	}

	void RefreshPosition()
	{
		if ( !m_Action_PositionX.IsFocused() && !m_Action_PositionX.IsEdited() ) m_Action_PositionX.SetText( FloatToString( target.GetPosition()[0] ) );
		if ( !m_Action_PositionY.IsFocused() && !m_Action_PositionY.IsEdited() ) m_Action_PositionY.SetText( FloatToString( target.GetPosition()[1] ) );
		if ( !m_Action_PositionZ.IsFocused() && !m_Action_PositionZ.IsEdited() ) m_Action_PositionZ.SetText( FloatToString( target.GetPosition()[2] ) );
	}

	void RefreshOrientation()
	{
		if ( !m_Action_OrientationX.IsFocused() && !m_Action_OrientationX.IsEdited() ) m_Action_OrientationX.SetText( FloatToString( target.GetOrientation()[0] ) );
		if ( !m_Action_OrientationY.IsFocused() && !m_Action_OrientationY.IsEdited() ) m_Action_OrientationY.SetText( FloatToString( target.GetOrientation()[1] ) );
		if ( !m_Action_OrientationZ.IsFocused() && !m_Action_OrientationZ.IsEdited() ) m_Action_OrientationZ.SetText( FloatToString( target.GetOrientation()[2] ) );
	}

	void Action_GetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard("<" + m_Action_PositionX.GetText() + ", " + m_Action_PositionY.GetText() + ", " + m_Action_PositionZ.GetText() + ">");
	}

	void Action_SetPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
		{
			if (!m_Action_AutoRefreshPosition.IsChecked())
				return;
			else if (eid == UIEvent.CHANGE)
				m_Action_AutoRefreshPosition.SetChecked(false);

			if ( eid != UIEvent.MOUSEWHEEL )
				return;
		}

		SetPosition();
	}

	void SetPosition()
	{
		vector pos;
		pos[0] = m_Action_PositionX.GetText().ToFloat();
		pos[1] = m_Action_PositionY.GetText().ToFloat();
		pos[2] = m_Action_PositionZ.GetText().ToFloat();

		m_Action_PositionX.SetEdited(false);
		m_Action_PositionY.SetEdited(false);
		m_Action_PositionZ.SetEdited(false);

		if (networkLow || networkHigh)
			module.SetPosition( pos, target );
	}

	void Action_PastePosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		GetGame().CopyFromClipboard(clipboard);

		vector pos = clipboard.BeautifiedToVector();

		if (pos != vector.Zero)
		{
			m_Action_PositionX.SetText(pos[0].ToString());
			m_Action_PositionY.SetText(pos[1].ToString());
			m_Action_PositionZ.SetText(pos[2].ToString());

			SetPosition();
		}
	}

	void Action_RefreshPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RefreshPosition();
	}

	void Click_AutoRefreshPosition( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
	}

	void Action_GetOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		GetGame().CopyToClipboard("<" + m_Action_OrientationX.GetText() + ", " + m_Action_OrientationY.GetText() + ", " + m_Action_OrientationZ.GetText() + ">");
	}

	void Action_SetOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
		{
			if (!m_Action_AutoRefreshOrientation.IsChecked())
				return;
			else if (eid == UIEvent.CHANGE)
				m_Action_AutoRefreshOrientation.SetChecked(false);

			if ( eid != UIEvent.MOUSEWHEEL )
				return;
		}

		SetOrientation();
	}

	void SetOrientation()
	{
		vector ori;
		ori[0] = m_Action_OrientationX.GetText().ToFloat();
		ori[1] = m_Action_OrientationY.GetText().ToFloat();
		ori[2] = m_Action_OrientationZ.GetText().ToFloat();

		m_Action_OrientationX.SetEdited(false);
		m_Action_OrientationY.SetEdited(false);
		m_Action_OrientationZ.SetEdited(false);

		if (networkLow || networkHigh)
			module.SetOrientation( ori, target );
	}

	void Action_PasteOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		string clipboard;
		GetGame().CopyFromClipboard(clipboard);

		vector ori = clipboard.BeautifiedToVector();

		if (ori != vector.Zero)
		{
			m_Action_OrientationX.SetText(ori[0].ToString());
			m_Action_OrientationY.SetText(ori[1].ToString());
			m_Action_OrientationZ.SetText(ori[2].ToString());

			SetOrientation();
		}
	}

	void Action_RefreshOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RefreshOrientation();
	}

	void Click_AutoRefreshOrientation( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
	}

	void Action_SetHealth( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		float health = action.GetText().ToFloat();
		if ( health == -1 )
			return;

		module.SetHealth( health, "", target );
	}

	void Action_Delete( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( IsMissionOffline() )
			module.DeleteObject( target );
		else
			module.DeleteObject( networkLow, networkHigh );
	}

	void Action_Heal( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ((target.IsInherited(Man) || target.IsInherited(DayZCreature)) && target.IsDamageDestroyed())
			COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_NOTIFICATION_ERROR_CANNOT_HEAL_DEAD_CREATURE" ) );
		else
			module.Heal( target );
	}
};


class JMESPMetaPlayer : JMESPMeta
{
	UIActionText m_Player_Name;
	UIActionText m_Player_GUID;
	UIActionText m_Player_Steam;
	//UIActionText m_Player_Speaking;

	override void CreateActions( Widget parent )
	{
		m_Player_Name = UIActionManager.CreateText( parent, "Name: ", "" );
		m_Player_GUID = UIActionManager.CreateText( parent, "GUID: ", "" );
		m_Player_Steam = UIActionManager.CreateText( parent, "Steam: ", "" );
		//m_Player_Speaking = UIActionManager.CreateText( parent, "Talking: ", "" );

		super.CreateActions( parent );
	}

	override void UpdateActions()
	{
		if ( !viewTypeActions || !widgetRoot || !player )
			return;

		super.UpdateActions();
		
		m_Player_Name.SetText( player.GetName() );
		m_Player_GUID.SetText( player.GetGUID() );
		m_Player_Steam.SetText( player.GetSteam64ID() );

		PlayerBase pb = PlayerBase.Cast(target);
		if (pb)
		{
			//if (pb.IsPlayerSpeaking() > 0.1) m_Player_Speaking.SetText("Yes");
			//else m_Player_Speaking.SetText("No");
		}
	}

	override bool CanDelete()
	{
		return !target.IsAlive();
	}
};


class JMESPMetaBaseBuilding : JMESPMeta
{
	ref map< string, ref JMConstructionPartData > m_Parts;

	ref array< UIActionText > m_StateHeaders;
	ref array< UIActionButton > m_BuildButtons;
	ref array< UIActionButton > m_DismantleButtons;
	ref array< UIActionButton > m_RepairButtons;

	BaseBuildingBase m_BaseBuilding;

	void JMESPMetaBaseBuilding()
	{
		m_Parts = new map< string, ref JMConstructionPartData >;

		m_StateHeaders = new array< UIActionText >;
		m_BuildButtons = new array< UIActionButton >;
		m_DismantleButtons = new array< UIActionButton >;
		m_RepairButtons = new array< UIActionButton >;
	}

	void ~JMESPMetaBaseBuilding()
	{
		if (!GetGame())
			return;

		if ( m_BaseBuilding )
		{
			m_BaseBuilding.m_COT_ConstructionUpdate.Remove( OnPartsUpdate );
		}
	}

	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

		m_BaseBuilding = BaseBuildingBase.Cast( target );

		m_BaseBuilding.m_COT_ConstructionUpdate.Insert( OnPartsUpdate );

		m_BaseBuilding.GetConstruction().COT_GetParts( m_Parts, false );

		for ( int i = 0; i < m_Parts.Count(); ++i )
		{
			string partName = m_Parts.GetKey( i );
			JMConstructionPartData part = m_Parts.Get( partName );

			Widget partActions = UIActionManager.CreateGridSpacer( parent, 2, 1 );

			UIActionText txt_H = UIActionManager.CreateText( partActions, part.m_DisplayName, "INIT" );

			Widget buttonActions = UIActionManager.CreateGridSpacer( partActions, 1, 3 );
			UIActionButton btn_B = UIActionManager.CreateButton( buttonActions, "Build", this, "Action_Build" );
			UIActionButton btn_D = UIActionManager.CreateButton( buttonActions, "Dismantle", this, "Action_Dismantle" );
			UIActionButton btn_R = UIActionManager.CreateButton( buttonActions, "Repair", this, "Action_Repair" );

			txt_H.SetData( part );
			btn_B.SetData( part );
			btn_D.SetData( part );
			btn_R.SetData( part );

			m_StateHeaders.Insert( txt_H );
			m_BuildButtons.Insert( btn_B );
			m_DismantleButtons.Insert( btn_D );
			m_RepairButtons.Insert( btn_R );

			//UIActionManager.CreatePanel( parent, 0xFF000000, 1 );
		}

		UpdateButtonStates();
	}

	void OnPartsUpdate()
	{
		m_BaseBuilding.GetConstruction().COT_GetParts( m_Parts, false );

		UpdateButtonStates();
	}

	private string StateToString( JMConstructionPartState state )
	{
		switch ( state )
		{
			case JMConstructionPartState.BUILT:
				return "Built";
			case JMConstructionPartState.CAN_BUILD:
				return "Can Build";
			case JMConstructionPartState.REQUIRED_PART_NOT_BUILT:
				return "Requires Part";
			case JMConstructionPartState.CONFLICTING_PART:
				return "Conflicting Part";
			case JMConstructionPartState.NOT_ENOUGH_MATERIALS:
				return "Not Enough Materials";
		}

		return "UNKNOWN";
	}

	private void UpdateButtonStates()
	{
		for ( int i = 0; i < m_StateHeaders.Count(); ++i )
		{
			JMConstructionPartData data;
			Class.CastTo( data, m_StateHeaders[i].GetData() );

			m_StateHeaders[i].SetText("");

			m_StateHeaders[i].Hide();
			m_BuildButtons[i].Hide();
			m_DismantleButtons[i].Hide();
			m_RepairButtons[i].Hide();

			if ( data.m_State == JMConstructionPartState.BUILT )
			{
				m_StateHeaders[i].Enable();
				m_StateHeaders[i].Show();

				m_RepairButtons[i].Enable();
				m_RepairButtons[i].Show();

				m_DismantleButtons[i].Enable();
				m_DismantleButtons[i].Show();
			}
			else if ( data.m_State != JMConstructionPartState.CONFLICTING_PART )
			{
				m_StateHeaders[i].Enable();
				m_StateHeaders[i].Show();
				
				m_BuildButtons[i].Enable();
				m_BuildButtons[i].Show();
			}
		}
	}

	void Action_Build( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Build( m_BaseBuilding, data.m_Name );
	}

	void Action_Dismantle( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Dismantle( m_BaseBuilding, data.m_Name );
	}

	void Action_Repair( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		JMConstructionPartData data;
		Class.CastTo( data, action.GetData() );

		module.BaseBuilding_Repair( m_BaseBuilding, data.m_Name );
	}
};

class JMESPMetaCar : JMESPMeta
{
	UIActionButton m_UnstuckButton;
	UIActionButton m_RefuelButton;

	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

		if (target.IsInherited(CarScript))
			m_UnstuckButton = UIActionManager.CreateButton( parent, "Unstuck", this, "Action_Unstuck" );
		m_RefuelButton  = UIActionManager.CreateButton( parent, "Refuel",  this, "Action_Refuel" );
	}

	void Action_Unstuck( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.Vehicle_Unstuck( target );
	}

	void Action_Refuel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.Vehicle_Refuel( target );
	}
};

class JMESPMetaBoat : JMESPMeta
{
	UIActionButton m_UnstuckButton;
	UIActionButton m_RefuelButton;

	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

		m_UnstuckButton = UIActionManager.CreateButton( parent, "Unstuck", this, "Action_Unstuck" );
		m_RefuelButton  = UIActionManager.CreateButton( parent, "Refuel",  this, "Action_Refuel" );
	}

	void Action_Unstuck( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.Vehicle_Unstuck( target );
	}

	void Action_Refuel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.Vehicle_Refuel( target );
	}
};

class JMESPMetaTrain : JMESPMeta
{
	UIActionButton m_RefuelButton;

	override void CreateActions( Widget parent )
	{
		super.CreateActions( parent );

		m_RefuelButton  = UIActionManager.CreateButton( parent, "Refuel",  this, "Action_Refuel" );
	}

	void Action_Refuel( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		module.Vehicle_Refuel( target );
	}
};