//! #define scope in Enforce is per-file, NOT per compiled module - see COTModule.c.
#define COT_DEBUGLOGS

enum JMESPState
{
	UNKNOWN = 0,
	Update,
	View,
	Remove
}

class JMESPCanvas
{
	protected CanvasWidget m_Canvas;

	void JMESPCanvas()
	{
		CreateCanvas();
	}

	void CreateCanvas()
	{
		if (!m_Canvas)
			m_Canvas = CanvasWidget.Cast(g_Game.GetWorkspace().CreateWidgets("JM/COT/GUI/layouts/esp_canvas.layout"));
	}

	bool HasCanvas()
	{
		if (m_Canvas && m_Canvas.ToString() != "INVALID")
			return true;

		return false;
	}

	void DrawLine(vector from, vector to, int width = 1, int color = COLOR_WHITE)
	{
		bool isInBoundsFrom;
		bool isInBoundsTo;
		from = TransformToScreenPos(from, isInBoundsFrom);
		to = TransformToScreenPos(to, isInBoundsTo);

		if (!isInBoundsFrom || !isInBoundsTo)
			return;

		m_Canvas.DrawLine(from[0], from[1], to[0], to[1], width, color);
	}

	void DrawCircle(vector center, float radius, int width = 1, int color = COLOR_WHITE, int segments = 36)
	{
		bool isInBounds;
		center = TransformToScreenPos(center, isInBounds);

		if (!isInBounds)
			return;

		float segmentLength = 360 / segments;

		for (int i = 0; i < segments; i++)
		{
			float a = i * segmentLength;

			float x1 = center[0] + (radius * Math.Cos(a * Math.DEG2RAD));
			float y1 = center[1] + (radius * Math.Sin(a * Math.DEG2RAD));

			float x2 = center[0] + (radius * Math.Cos((a + segmentLength) * Math.DEG2RAD));
			float y2 = center[1] + (radius * Math.Sin((a + segmentLength) * Math.DEG2RAD));

			m_Canvas.DrawLine(x1, y1, x2, y2, width, color);
		}
	}

	void Clear()
	{
		m_Canvas.Clear();
	}

	vector TransformToScreenPos(vector pWorldPos, out bool isInBounds = false)
	{
		float parent_width, parent_height;
		vector screen_pos;
		
		//! get relative pos for screen from world pos vector
		screen_pos = g_Game.GetScreenPosRelative(pWorldPos);
		isInBounds = screen_pos[0] >= 0 && screen_pos[0] <= 1 && screen_pos[1] >= 0 && screen_pos[1] <= 1 && screen_pos[2] >= 0;
		//! get size of parent widget
		m_Canvas.GetScreenSize(parent_width, parent_height);
		
		//! calculate current position from relative pos and parent widget size
		screen_pos[0] = screen_pos[0] * parent_width;
		screen_pos[1] = screen_pos[1] * parent_height;
		
		return screen_pos;
	}
}

class JMESPLimb
{
	string Bone1;
	string Bone2;

	void JMESPLimb(string bone1, string bone2)
	{
		Bone1 = bone1;
		Bone2 = bone2;
	}
}

class JMESPSkeleton
{
	protected static ref array<ref JMESPLimb> s_Limbs = InitLimbs();

	protected static array<ref JMESPLimb> InitLimbs()
	{
		auto limbs = new array<ref JMESPLimb>();

		limbs.Insert(new JMESPLimb("neck", "spine3"));
		limbs.Insert(new JMESPLimb("spine3", "pelvis"));
		limbs.Insert(new JMESPLimb("neck", "leftarm"));
		limbs.Insert(new JMESPLimb("leftarm", "leftforearm"));
		limbs.Insert(new JMESPLimb("leftforearm", "lefthand"));
		limbs.Insert(new JMESPLimb("lefthand", "lefthandmiddle4"));
		limbs.Insert(new JMESPLimb("pelvis", "leftupleg"));
		limbs.Insert(new JMESPLimb("leftupleg", "leftleg"));
		limbs.Insert(new JMESPLimb("leftleg", "leftfoot"));
		limbs.Insert(new JMESPLimb("neck", "rightarm"));
		limbs.Insert(new JMESPLimb("rightarm", "rightforearm"));
		limbs.Insert(new JMESPLimb("rightforearm", "righthand"));
		limbs.Insert(new JMESPLimb("righthand", "righthandmiddle4"));
		limbs.Insert(new JMESPLimb("pelvis", "rightupleg"));
		limbs.Insert(new JMESPLimb("rightupleg", "rightleg"));
		limbs.Insert(new JMESPLimb("rightleg", "rightfoot"));

		return limbs;
	}

	static void Draw(Human human, JMESPCanvas canvas = null, float lineThickness = 1, out array<Shape> shapes = null)
	{
#ifdef JM_COT_USE_DEBUGSHAPES
		shapes = new array<Shape>;
#endif

		int color = COLOR_WHITE;
		switch (human.GetHealthLevel())
		{
			case GameConstants.STATE_PRISTINE:
			case GameConstants.STATE_WORN:
				break;
			case GameConstants.STATE_DAMAGED:
				color = 0xFFDCDC00;
				break;
			case GameConstants.STATE_BADLY_DAMAGED:
				color = 0xFFDC0000;
				break;
			case GameConstants.STATE_RUINED:
				color = 0xFF232323;
				break;
		}

		vector neckPos = human.GetBonePositionWS(human.GetBoneIndexByName("neck"));
		vector headPos = human.GetBonePositionWS(human.GetBoneIndexByName("head"));

		vector dir = vector.Direction(neckPos, headPos).Normalized() * human.GetScale();
		vector neckEnd = headPos - dir * 0.06;

		//! Neck
#ifdef JM_COT_USE_DEBUGSHAPES
		shapes.Insert(Debug.DrawLine(neckPos, neckEnd, color, ShapeFlags.NOZBUFFER));
#else
		canvas.DrawLine(neckPos, neckEnd, lineThickness, color);
#endif

		//! Head
		headPos = headPos + dir * 0.06;
		float radius = 0.12 * human.GetScale();
#ifdef JM_COT_USE_DEBUGSHAPES
		shapes.Insert(Debug.DrawSphere(headPos, radius, color, ShapeFlags.WIREFRAME | ShapeFlags.NOZBUFFER));
#else
		bool isInBounds;
		vector p1 = canvas.TransformToScreenPos(headPos, isInBounds);
		if (isInBounds && p1[0] > 0 && p1[1] > 0 && p1[2] > radius)
		{
			vector ori = g_Game.GetCurrentCameraDirection().VectorToAngles();
			ori[1] = 0;
			ori[2] = 0;
			vector p2 = canvas.TransformToScreenPos(headPos + ori.AnglesToVector().Perpend() * radius);
			float dist = vector.Distance(p1, p2);
			canvas.DrawCircle(headPos, dist, lineThickness, color);
		}
#endif

		foreach (JMESPLimb limb: s_Limbs)
		{
			vector bone1Position = human.GetBonePositionWS(human.GetBoneIndexByName(limb.Bone1));
			vector bone2Position = human.GetBonePositionWS(human.GetBoneIndexByName(limb.Bone2));
			if (limb.Bone2.Contains("foot"))
				bone2Position = bone2Position + vector.Direction(bone1Position, bone2Position).Normalized() * 0.1;
#ifdef JM_COT_USE_DEBUGSHAPES
			shapes.Insert(Debug.DrawLine(bone1Position, bone2Position, color, ShapeFlags.NOZBUFFER));
#else
			canvas.DrawLine(bone1Position, bone2Position, lineThickness, color);
#endif
		}
	}
}

class JMESPModule: JMRenderableModuleBase
{
	private ref array< Object > m_SelectedObjects;
	
	private ref array< ref JMESPMeta > m_ActiveESPObjects;

	private ref array< ref JMESPMeta > m_ESPToCreate;
	private ref array< JMESPMeta > m_ESPToDestroy;

	//! protected, not private: DayZ-Expansion's ExpansionESPModificationModule
	//! (`modded class JMESPModule`) reads this via m_MappedESPObjects.Get(target).
	//! A modded class cannot touch a private member of the class it mods.
	protected ref map< Object, JMESPMeta > m_MappedESPObjects;

	private ref array< ref JMESPViewType > m_ViewTypes;
	private ref map<typename, JMESPViewType> m_ViewTypesByType;

	//! Client-side per-category colour overrides. Null on a dedicated server -
	//! nothing there draws an overlay, and it would only create an empty file.
	private ref JMESPSerialize m_ColourSettings;

	private bool m_IsCreatingWidgets;
	private bool m_IsDestroyingWidgets;
	private bool m_IknowWhatIamDoing;

	//! Right-click-anything, for as long as COT's own interface is up.
	//!
	//! The button is polled rather than handled, because there is no widget
	//! between the cursor and the world to hand a click to - the ESP container
	//! ignores the pointer so the tags under it stay clickable, and a catcher
	//! panel wide enough to hear the click would swallow every one of theirs.
	private bool m_WorldMenuRightDown;

	//! The object picked out of the world, kept alive here.
	//!
	//! A tag's meta is owned by the tracking lists; one made for a right-click
	//! is owned by nothing, and the menu holds it weakly.
	private ref JMESPMeta m_WorldMenuMeta;

	//! How far a right-click reaches. Past this it is scenery an admin is
	//! looking at rather than something they meant to act on.
	static const float WORLD_MENU_RANGE = 200;

	//! Bounds on the one RPC that turns a client's string into a line in the
	//! admin log and a webhook post. The real sender writes a short fixed
	//! phrase when the ESP view is switched, so neither of these is reachable
	//! by anything legitimate.
	static const int LOG_MAX_LENGTH = 256;
	static const int LOG_MIN_INTERVAL_MS = 1000;

	//! How often a move or a turn is written down, per admin.
	//!
	//! Dragging a tag streams a position up to ten times a second, and each of
	//! those used to append a line to the admin log and post a webhook - so a
	//! ten second drag was a hundred file writes and a hundred HTTP requests,
	//! which is enough to get a Discord endpoint rate limited by ordinary use,
	//! never mind by a client sending them deliberately.
	//!
	//! The move itself still applies every time. Only the record of it is
	//! sampled, which is what an audit trail of a continuous drag wants anyway.
	static const int MOVE_RECORD_INTERVAL_MS = 2000;

	string Filter;

	float ESPRadius;
	int ESPUpdateTime;
	private bool DrawPlayerSkeletonsEnabled;
	bool DrawPlayerSkeletonsIncludingMyself;
	float SkeletonLineThickness = 1;

	private JMESPState m_CurrentState = JMESPState.Remove;
	private bool m_StateChanged = false;
	bool m_RemoveDeleted;

	ref JMESPCanvas m_ESPCanvas;

	private JMLoadoutModule m_LoadoutModule;

	//! What a fireplace is refuelled with. Firewood rather than sticks: it is
	//! the highest-energy fuel every fireplace accepts, so one stack is a fire
	//! that stays lit long enough to be worth lighting.
	static const string FUEL_TYPE = "Firewood";

	//! How far a lock-all reaches, in metres. Wide enough to take in a whole
	//! compound from a gate in its wall, short enough that it cannot reach a
	//! neighbour's base across a street.
	static const float LOCK_ALL_RADIUS = 40.0;

	void JMESPModule()
	{
		ESPRadius = 200;

		ESPUpdateTime = 5;

		GetPermissionsManager().RegisterPermission( "ESP.View" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.SetPosition" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetOrientation" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetHealth" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Delete" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.DuplicateAll" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.DeleteAll" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.MoveToCursor" );		

		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Build" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Build.MaterialsNotRequired" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Dismantle" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.Repair" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.BaseBuilding.SetHealth" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.Car.Unstuck" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Car.Refuel" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Car.LockWheels" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.Heal" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.ChangeColor" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.SetQuantity" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetLiquid" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetCleanness" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetFoodStage" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.OpenClose" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.Weapon" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetAttachment" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Fireplace" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.Lock" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.SetCode" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.GetCode" );

		//! New children of ESP.Object, so a role that already allows the parent
		//! allows these too - an unlisted permission inherits.
		GetPermissionsManager().RegisterPermission( "ESP.Object.ClearCargo" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Immobilize" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Trap" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Flag" );
	}

#ifdef SERVER
	override void EnableUpdate()
	{
	}
#endif

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

	override string GetCategory()
	{
		return "World";
	}

	override string GetTitle()
	{
		return "#STR_COT_ESP_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return JMConstants.Lucide( "scan-eye" );
	}

	override bool ImageIsIcon()
	{
		return true;
	}

	override bool ImageHasPath()
	{
		return true;
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
		types.Insert( "Vehicle_Unstuck" );
		types.Insert( "Repair" );
		types.Insert( "Vehicle_Refuel" );
		types.Insert( "MakeItemSet" );
		types.Insert( "DuplicateAll" );
		types.Insert( "DeleteAll" );
		types.Insert( "MoveToCursor" );
	}

	override void RegisterKeyMouseBindings()
	{
		super.RegisterKeyMouseBindings();
		Bind( new JMModuleBinding( "Input_ESP_DeleteCursor",  "UAESPModuleDeleteCursor",  true ) );
		Bind( new JMModuleBinding( "Input_ESP_HealCursor",    "UAESPModuleHealCursor",    true ) );
		Bind( new JMModuleBinding( "Input_ESP_MoveToCursor",  "UAESPModuleMoveToCursor",  true ) );
	}

	void Input_ESP_DeleteCursor( UAInput input )
	{
		if ( !input.LocalPress() ) return;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Delete" ) ) return;
		Object obj = GetESPObjectAtCursor();
		if ( !obj ) return;
		int low, high;
		obj.GetNetworkID( low, high );
		DeleteObject( low, high );
	}

	void Input_ESP_HealCursor( UAInput input )
	{
		if ( !input.LocalPress() ) return;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Heal" ) ) return;
		Object obj = GetESPObjectAtCursor();
		if ( obj ) Heal( obj );
	}

	void Input_ESP_MoveToCursor( UAInput input )
	{
		if ( !input.LocalPress() ) return;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.MoveToCursor" ) ) return;
		vector dir = g_Game.GetCurrentCameraDirection();
		vector from = g_Game.GetCurrentCameraPosition();
		vector to = from + ( dir * 1000 );
		vector contact_pos;
		vector contact_dir;
		int contact_component;
		if ( DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, NULL, NULL, NULL, false, true ) )
			MoveToCursor( contact_pos );
	}

	protected Object GetESPObjectAtCursor()
	{
		vector rayStart = g_Game.GetCurrentCameraPosition();
		vector rayEnd = rayStart + ( g_Game.GetCurrentCameraDirection() * 50 );
		RaycastRVParams rayInput = new RaycastRVParams( rayStart, rayEnd );
		rayInput.flags = CollisionFlags.ALLOBJECTS;
		rayInput.radius = 0.15;
		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;
		if ( !DayZPhysics.RaycastRVProxy( rayInput, results ) ) return NULL;
		PlayerBase selfPlayer = PlayerBase.Cast( g_Game.GetPlayer() );
		bool allowSelf = selfPlayer && selfPlayer.IsInThirdPerson();

		foreach ( RaycastRVResult res: results )
		{
			if ( !res.obj )
				continue;

			if ( res.obj == g_Game.GetPlayer() && !allowSelf )
				continue;

			return res.obj;
		}
		return NULL;
	}

	override void OnClientPermissionsUpdated()
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_0(this, "OnClientPermissionsUpdated");
		#endif

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
		EnableLogout();
		
		m_SelectedObjects = new array< Object >;

		m_ActiveESPObjects = new array< ref JMESPMeta >;

		m_ESPToCreate = new array< ref JMESPMeta >;
		m_ESPToDestroy = new array< JMESPMeta >;

		m_MappedESPObjects = new map< Object, JMESPMeta >;

		m_ViewTypes = new array< ref JMESPViewType >;
		m_ViewTypesByType = new map<typename, JMESPViewType>;

		TTypenameArray espTypes = new TTypenameArray;
		RegisterTypes( espTypes );
		
		foreach (typename espType: espTypes)
		{
			if ( espType.IsInherited( JMESPViewType ) )
			{
				JMESPViewType viewType = JMESPViewType.Cast( espType.Spawn() );
				if ( viewType )
				{
					m_ViewTypes.Insert( viewType );
					m_ViewTypesByType[espType] = viewType;
					GetPermissionsManager().RegisterPermission( "ESP.View." + viewType.Permission );
				}
			}
		}

		if ( !g_Game.IsDedicatedServer() )
		{
			m_ColourSettings = JMESPSerialize.Load();
			m_ColourSettings.Apply( m_ViewTypes );
		}
	}

	void RegisterTypes( out TTypenameArray types )
	{
		types.Insert( JMESPViewTypePlayer );
		types.Insert( JMESPViewTypePlayerAI );
		types.Insert( JMESPViewTypeInfected );
		types.Insert( JMESPViewTypeAnimal );

		types.Insert( JMESPViewTypeCar );
		types.Insert( JMESPViewTypeBoat );

		if (CommunityOnlineToolsBase.s_HypeTrain_Loco_Type)
			types.Insert( JMESPViewTypeTrain );
		
		types.Insert( JMESPViewTypeArchery );
		types.Insert( JMESPViewTypeBoltActionRifle );
		types.Insert( JMESPViewTypeBoltRifle );
		types.Insert( JMESPViewTypeRifle );
		types.Insert( JMESPViewTypePistol );
		types.Insert( JMESPViewTypeLauncher );

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
		types.Insert( JMESPViewTypeUnknown );

		types.Insert( JMESPViewTypeBuilding );
		types.Insert( JMESPViewTypeRock );
		types.Insert( JMESPViewTypePlainObject );
		types.Insert( JMESPViewTypeTree );
		types.Insert( JMESPViewTypeBush );
		types.Insert( JMESPViewTypeImmovable );
	}

	array< ref JMESPViewType > GetViewTypes()
	{
		return m_ViewTypes;
	}

	//! The objects currently tracked and drawn. The form's object list reads
	//! this rather than walking JMESPMeta.s_JM_All: the linked list also holds
	//! metas queued for creation and destruction, which are not on screen yet.
	array< ref JMESPMeta > GetActiveObjects()
	{
		return m_ActiveESPObjects;
	}

	//! Persist a category colour edit made from the filter list. No-op on a
	//! dedicated server, where the overrides were never loaded.
	void SetViewTypeColour( JMESPViewType viewType, int colour )
	{
		if ( !viewType )
			return;

		viewType.Colour = colour;

		if ( m_ColourSettings )
			m_ColourSettings.SetColour( viewType.Permission, colour );
	}

	//! Drop one category's override. Same trick as the bulk reset: the default
	//! only exists in the subclass constructor, so a fresh instance is how it
	//! is read back.
	void ResetViewTypeColour( JMESPViewType viewType )
	{
		if ( !viewType )
			return;

		if ( m_ColourSettings )
			m_ColourSettings.ClearColour( viewType.Permission );

		JMESPViewType defaults = JMESPViewType.Cast( viewType.Type().Spawn() );

		if ( defaults )
			viewType.Colour = defaults.Colour;
	}

	//! Drop every override and put the constructor defaults back. Spawning a
	//! fresh instance is how the default is read - it is only ever written in
	//! the subclass constructor.
	void ResetViewTypeColours()
	{
		if ( m_ColourSettings )
			m_ColourSettings.ClearColours();

		foreach ( JMESPViewType viewType : m_ViewTypes )
		{
			JMESPViewType defaults = JMESPViewType.Cast( viewType.Type().Spawn() );

			if ( defaults )
				viewType.Colour = defaults.Colour;
		}
	}

	JMESPViewType GetViewType(typename type)
	{
		return m_ViewTypesByType[type];
	}

	bool IncludeImmovable()
	{
		JMESPViewType vt;

		if ( m_ViewTypesByType.Contains( JMESPViewTypePlainObject ) )
		{
			vt = m_ViewTypesByType[ JMESPViewTypePlainObject ];
			if ( vt && vt.View ) return true;
		}

		if ( m_ViewTypesByType.Contains( JMESPViewTypeRock ) )
		{
			vt = m_ViewTypesByType[ JMESPViewTypeRock ];
			if ( vt && vt.View ) return true;
		}

		if ( m_ViewTypesByType.Contains( JMESPViewTypeBush ) )
		{
			vt = m_ViewTypesByType[ JMESPViewTypeBush ];
			if ( vt && vt.View ) return true;
		}

		if ( m_ViewTypesByType.Contains( JMESPViewTypeTree ) )
		{
			vt = m_ViewTypesByType[ JMESPViewTypeTree ];
			if ( vt && vt.View ) return true;
		}

		if ( m_ViewTypesByType.Contains( JMESPViewTypeImmovable ) )
		{
			vt = m_ViewTypesByType[ JMESPViewTypeImmovable ];
			if ( vt && vt.View ) return true;
		}

		return false;
	}

	override void OnMissionStart()
	{
		super.OnMissionStart();

		if ( IsMissionClient() )
		{
			JMESPWidgetHandler.espModule = this;
		}
	}

	override void OnMPSessionFail()
	{
	#ifdef DIAG
		Print("JMESPModule::OnMPSessionFail");
	#endif

		super.OnMPSessionFail();

		g_COT_ThreadESP = false;
	}

	override void OnMPSessionEnd()
	{
	#ifdef DIAG
		Print("JMESPModule::OnMPSessionEnd");
	#endif

		super.OnMPSessionEnd();

		g_COT_ThreadESP = false;
	}

	override void OnMPConnectionLost(int duration)
	{
	#ifdef DIAG
		PrintFormat("JMESPModule::OnMPConnectionLost duration=%1", duration);
	#endif

		super.OnMPConnectionLost(duration);
	}

	override void OnMissionFinish()
	{
	#ifdef DIAG
		Print("JMESPModule::OnMissionFinish");
	#endif

		for (int j = 0; j < m_ActiveESPObjects.Count(); j++ )
		{
			m_ActiveESPObjects[j].Destroy();
		}

		m_ActiveESPObjects.Clear();

		g_COT_ThreadESP = false;
		g_COT_ThreadESP_Running = false;
	}

	override void OnLogout(Class sender, CF_EventArgs args)
	{
	#ifdef DIAG
		Print("JMESPModule::OnLogout");
	#endif

		g_COT_ThreadESP = false;
		m_CurrentState = JMESPState.Remove;
	}

	void CreateCanvas()
	{
		if (!m_ESPCanvas)
			m_ESPCanvas = new JMESPCanvas();
		else
			m_ESPCanvas.CreateCanvas();
	}

	bool GetFilterSafetyState()
	{
		return m_IknowWhatIamDoing;
	}

	void SetFilterSafetyState(bool state)
	{
		m_IknowWhatIamDoing = state;
	}

	float GetMaxRadius()
	{
		if (m_ViewTypesByType[JMESPViewTypeBush].View || m_ViewTypesByType[JMESPViewTypeTree].View)
			return 300;

		return 1000;
	}

	// =========================================================================
	//  World context menu
	//
	//  The menu the ESP tags and the tracked list already raise, over whatever
	//  the cursor is pointing at - tracked or not, ESP drawing or not. It only
	//  listens while COT's own interface is up, because that is the only time
	//  the cursor belongs to the admin rather than to the game.
	// =========================================================================

	protected void UpdateWorldContextMenu()
	{
		if ( !IsMissionClient() )
			return;

		bool down = ( GetMouseState( MouseState.RIGHT ) & MB_PRESSED_MASK ) != 0;

		//! The edge, not the level: a held button is a drag happening somewhere
		//! else, not a menu being asked for again every frame.
		if ( down == m_WorldMenuRightDown )
			return;

		m_WorldMenuRightDown = down;

		if ( !down )
			return;

		if ( !IsCOTInterfaceOpen() )
			return;

		if ( !HasAccess() )
			return;

		//! A press that landed on COT's own UI belongs to whatever it landed on
		//! - a window, the sidebar, an ESP tag, or the open menu itself, which
		//! closes through its own handler.
		if ( IsCursorOverCOTUI() )
			return;

		OpenWorldContextMenu();
	}

	//! Is COT on screen AND holding the mouse?
	//!
	//! Both halves of the first test matter: the sidebar can be up with no
	//! window open, and a pinned window can be up with the sidebar hidden.
	//!
	//! The cursor test is the one that makes this safe. COT hands the mouse
	//! back to the game when a click lands outside its own UI, and a pinned
	//! window stays on screen through that - so "a window is visible" is not
	//! the same question as "is the pointer the admin's". Without this, a
	//! right-click meant as aim down sights would raise a menu nobody could
	//! click.
	protected bool IsCOTInterfaceOpen()
	{
		if ( !g_Game.GetUIManager() || !g_Game.GetUIManager().IsCursorVisible() )
			return false;

		//! A game menu of its own - inventory, the escape menu - is holding the
		//! cursor, not COT. It gets its own right-clicks.
		if ( g_Game.GetUIManager().GetMenu() )
			return false;

		if ( GetCommunityOnlineToolsBase() && GetCommunityOnlineToolsBase().IsOpen() )
			return true;

		if ( GetCOTWindowManager() && GetCOTWindowManager().HasAnyActive() )
			return true;

		return false;
	}

	//! Whether the thing under the cursor is COT's rather than the world's.
	//!
	//! Walked up the tree rather than tested directly: the widget the cursor is
	//! actually over is a label or a panel deep inside a window, and only its
	//! ancestors say which window - or which tag - it belongs to.
	protected bool IsCursorOverCOTUI()
	{
		Widget under = GetWidgetUnderCursor();

		while ( under )
		{
			if ( under == JMStatics.ESP_CONTAINER )
				return true;

			if ( under == JMStatics.COT_MENU )
				return true;

			if ( JMStatics.IsOverlay( under ) )
				return true;

			if ( GetCOTWindowManager() && GetCOTWindowManager().GetWindowFromWidget( under ) )
				return true;

			under = under.GetParent();
		}

		return false;
	}

	protected void OpenWorldContextMenu()
	{
		JMESPActionMenu menu = JMESPActionMenu.Shared();

		if ( !menu )
			return;

		int component;
		Object obj = PickWorldObject( component );

		if ( !obj )
			return;

		JMESPMeta meta;

		if ( !CreateMetaForObject( obj, meta ) )
			return;

		//! Static map buildings only get door actions when the click actually
		//! landed on one of their doors. GetDoorIndex needs a VIEW geometry
		//! component specifically - PickWorldObject's component is whichever
		//! of fire/view/geom answered first (fire usually wins on a building),
		//! which is the wrong index space and silently maps to the wrong door
		//! (or none at all), so this re-picks with a dedicated view-only ray.
		Building building = Building.Cast( obj );
		if ( building )
			meta.m_DoorIndex = GetBuildingDoorIndexAtCursor( building );
		else
			meta.m_DoorIndex = -1;

		m_WorldMenuMeta = meta;

		int mx;
		int my;
		GetMousePos( mx, my );

		menu.Open( meta, mx, my );
	}

	//! What the mouse is pointing at.
	//!
	//! The pointer's direction, not the camera's: with the interface open the
	//! cursor moves independently of where the camera looks, and a ray down the
	//! camera axis would answer for the middle of the screen instead of for the
	//! thing that was clicked.
	//!
	//! Three intersection types in order, because one alone misses things an
	//! admin can plainly see: fire geometry finds most entities, view geometry
	//! finds foliage and anything with no fire volume, and plain geometry finds
	//! the rest.
	//! Insertion sort - result lists here are only ever a handful of entries.
	protected void SortRaycastResultsByDistance( array< ref RaycastRVResult > results, vector from )
	{
		for ( int i = 1; i < results.Count(); i++ )
		{
			ref RaycastRVResult current = results[i];
			float currentDist = vector.Distance( from, current.pos );
			int j = i - 1;

			while ( j >= 0 && vector.Distance( from, results[j].pos ) > currentDist )
			{
				results[j + 1] = results[j];
				j--;
			}

			results[j + 1] = current;
		}
	}

	//! GetDoorIndex only means anything against a VIEW geometry component -
	//! this fires its own ray, on the same axis PickWorldObject used, with
	//! type pinned to ObjIntersectView so the component it reads is in the
	//! space GetDoorIndex actually expects. Returns -1 if the view ray does
	//! not land on this same building (a wall gap, the roof, ...) at all.
	protected int GetBuildingDoorIndexAtCursor( Building building )
	{
		if ( !building )
			return -1;

		vector from = g_Game.GetCurrentCameraPosition();
		vector dir = g_Game.GetPointerDirection();

		if ( dir.Length() < 0.01 )
			dir = g_Game.GetCurrentCameraDirection();

		vector to = from + ( dir * WORLD_MENU_RANGE );

		RaycastRVParams rayInput = new RaycastRVParams( from, to );
		rayInput.flags = CollisionFlags.ALLOBJECTS;
		rayInput.radius = 0.1;
		rayInput.type = ObjIntersectView;

		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		if ( !DayZPhysics.RaycastRVProxy( rayInput, results ) )
			return -1;

		SortRaycastResultsByDistance( results, from );

		foreach ( RaycastRVResult result: results )
		{
			Object obj = result.obj;
			if ( !obj )
				continue;

			EntityAI entity;
			if ( Class.CastTo( entity, obj ) && entity.GetHierarchyRoot() )
				obj = entity.GetHierarchyRoot();

			if ( obj != building )
				continue;

			return building.GetDoorIndex( result.component );
		}

		return -1;
	}

	protected Object PickWorldObject( out int outComponent )
	{
		outComponent = -1;

		vector from = g_Game.GetCurrentCameraPosition();
		vector dir = g_Game.GetPointerDirection();

		if ( dir.Length() < 0.01 )
			dir = g_Game.GetCurrentCameraDirection();

		vector to = from + ( dir * WORLD_MENU_RANGE );

		RaycastRVParams rayInput = new RaycastRVParams( from, to );
		rayInput.flags = CollisionFlags.ALLOBJECTS;
		rayInput.radius = 0.1;

		array< ref RaycastRVResult > results = new array< ref RaycastRVResult >;

		TIntArray types = { ObjIntersectFire, ObjIntersectView, ObjIntersectGeom };

		foreach ( int type: types )
		{
			rayInput.type = type;

			if ( !DayZPhysics.RaycastRVProxy( rayInput, results ) )
				continue;

			//! RaycastRVProxy does not guarantee hit order, so an overlapping
			//! far object could otherwise win over a nearer one of the same
			//! intersection type - sort by distance from the cursor ray origin
			//! so the closest hit is always tried first.
			SortRaycastResultsByDistance( results, from );

			foreach ( RaycastRVResult result: results )
			{
				Object obj = result.obj;

				if ( !obj )
					continue;

				//! An attachment or a cargo item answers the ray, but the thing
				//! the admin pointed at is what is holding it.
				EntityAI entity;
				if ( Class.CastTo( entity, obj ) && entity.GetHierarchyRoot() )
					obj = entity.GetHierarchyRoot();

				//! In first person the camera sits inside the admin's own
				//! collision mesh, so the ray answers with self before it ever
				//! reaches whatever is actually under the cursor - skip it. In
				//! third person the camera is well clear of the body, and a
				//! self-hit there is the admin deliberately pointing at their
				//! own character; that is a legitimate right-click target.
				PlayerBase selfPlayer = PlayerBase.Cast( g_Game.GetPlayer() );
				if ( obj == g_Game.GetPlayer() && ( !selfPlayer || !selfPlayer.IsInThirdPerson() ) )
					continue;

				COT_String objType = JMESPMeta.GetObjectType( obj );
				objType.ToLower();

				if ( objType == "#particlesourceenf" )
					continue;

				outComponent = result.component;
				return obj;
			}
		}

		return NULL;
	}

	//! A meta for an object nobody is tracking.
	//!
	//! Built through the view types rather than by a type switch here: they
	//! already know which meta subclass an object needs - and third party mods
	//! add their own - so the menu a right-click raises offers exactly the pages
	//! it would have offered on a tag.
	//!
	//! The permission each view type carries is respected, but its View toggle
	//! is not: the toggles say what to DRAW, and this draws nothing.
	//!
	//! Deliberately not put in m_MappedESPObjects - that map belongs to the ESP
	//! worker thread, and an entry it did not make would be torn out from under
	//! it on the next pass.
	bool CreateMetaForObject( Object obj, out JMESPMeta meta )
	{
		if ( !obj )
			return false;

		//! Already tracked: use the meta the tag is using, so selecting from
		//! the menu lights up the tag on screen and deselecting from either
		//! place means the same thing.
		if ( m_MappedESPObjects )
		{
			JMESPMeta tracked = m_MappedESPObjects.Get( obj );

			if ( tracked )
			{
				meta = tracked;
				return true;
			}
		}

		array< ref JMESPViewType > viewTypes = GetViewTypes();

		if ( viewTypes )
		{
			for ( int i = 0; i < viewTypes.Count(); ++i )
			{
				if ( !viewTypes[i].HasPermission )
					continue;

				if ( viewTypes[i].IsValid( obj, meta ) )
				{
					meta.module = this;
					return true;
				}
			}
		}

		//! Nothing claimed it - every matching view type is denied, or it is
		//! something no view type describes. The menu still works off the target
		//! alone, so it gets a bare meta rather than nothing.
		if ( !meta )
			meta = new JMESPMeta;

		meta.target = obj;
		meta.module = this;
		meta.colour = JMTheme.INK_50;

		obj.GetNetworkID( meta.networkLow, meta.networkHigh );

		meta.name = meta.GetName();

		return true;
	}

	override void OnUpdate(float timeslice)
	{
		UpdateWorldContextMenu();

		if (!DrawPlayerSkeletonsEnabled || !m_ESPCanvas || !m_ESPCanvas.HasCanvas())
			return;

		auto spectatorCamera = JMSpectatorCamera.Cast(CurrentActiveCamera);
		Man gamePlayer = g_Game.GetPlayer();

		m_ESPCanvas.Clear();

		foreach (Man player : ClientData.m_PlayerBaseList)
		{
			Human human;
			if (!Class.CastTo(human, player))
				continue;

			if (JMESPViewType.IsPlayer(human))
			{
				if (!m_ViewTypesByType[JMESPViewTypePlayer].View)
					continue;
			}
			else if (!m_ViewTypesByType[JMESPViewTypePlayerAI].View)
			{
				continue;
			}

			if (spectatorCamera && spectatorCamera.SelectedTarget == player && !spectatorCamera.m_JM_3rdPerson)
				continue;

			if (player == gamePlayer && !DrawPlayerSkeletonsIncludingMyself)
				continue;

			vector btm = g_Game.GetScreenPosRelative(human.GetPosition());
			if (btm[2] < 0 || btm[2] > ESPRadius)
				continue;

			vector headPos = human.GetBonePositionWS(human.GetBoneIndexByName("head"));
			vector top = g_Game.GetScreenPosRelative(headPos);
			if (top[2] < 0.18)
				continue;

			btm[2] = btm[1];
			top[2] = top[1];
			if (!Math.IsPointInRectangle("0 0 0", "1 0 1", btm) && !Math.IsPointInRectangle("0 0 0", "1 0 1", top))
				continue;

			JMESPSkeleton.Draw(human, m_ESPCanvas, SkeletonLineThickness);
		}
	}

	void SetDrawPlayerSkeletonsEnabled(bool state)
	{
		if (!HasAccess())
			return;

		DrawPlayerSkeletonsEnabled = state;

		if (!state)
			m_ESPCanvas.Clear();
	}

	bool GetDrawPlayerSkeletonsEnabled()
	{
		return DrawPlayerSkeletonsEnabled;
	}

	private void CreateNewWidgets()
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPModule::CreateNewWidgets() void;" );
		#endif
		#endif

		if (!g_COT_ThreadESP_Running)
		{
			m_IsCreatingWidgets = false;
			return;
		}

		m_IsCreatingWidgets = true;

		int count = m_ESPToCreate.Count();

		if (count > 10)
			count = 10;

		for ( int i = count - 1; i >= 0; i-- )
		{
			JMESPMeta meta = m_ESPToCreate[i];

			meta.Create( this );

			m_ActiveESPObjects.Insert( meta );

			m_ESPToCreate.Remove(i);
		}
		
		if (m_ESPToCreate.Count() > 0)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CreateNewWidgets, 10, false);
		else
			m_IsCreatingWidgets = false;

		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "-JMESPModule::CreateNewWidgets() void;" );
		#endif
		#endif
	}

	private void DestroyOldWidgets()
	{
		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "+JMESPModule::DestroyOldWidgets() void;" );
		#endif
		#endif

		m_IsDestroyingWidgets = true;

		int count = m_ESPToDestroy.Count();

		if (count > 100)
			count = 100;

		for ( int i = count - 1; i >= 0; i-- )
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

				meta.Destroy();
			}

			m_ESPToDestroy.Remove(i);
		}

		#ifdef JM_COT_ESP_DEBUG
		Print( "  Clearing m_ESPToDestroy" );
		#endif

		if (m_ESPToDestroy.Count() > 0)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DestroyOldWidgets, 10, false);
		else
			m_IsDestroyingWidgets = false;

		if (m_CurrentState == JMESPState.Remove)
			g_COT_ThreadESP = false;

		#ifdef JM_COT_ESP_DEBUG
		#ifdef COT_DEBUGLOGS
		Print( "-JMESPModule::DestroyOldWidgets() void;" );
		#endif
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

	private void ChunkGetObjects(out set<Object> objects, out int totalTimeTaken)
	{
		if (!ESPRadius)
			return;

		vector centerPosition = GetCurrentPosition();

		float maxSizePerBox = 100;
		float sizePerBox = ESPRadius / maxSizePerBox;
		
		int numIterations = Math.Ceil(sizePerBox / 2);

		sizePerBox = (sizePerBox / numIterations) * maxSizePerBox;

		int sleepIdx = 0;

		bool includeImmovable;
		bool includeBushes;
		bool includeCreatures;

		int flags = QueryFlags.DYNAMIC;

		if (m_ViewTypesByType[JMESPViewTypeBush].View)
		{
			includeBushes = true;
		}
		else if (IncludeImmovable())
		{
			includeImmovable = true;

			if (m_ViewTypesByType[JMESPViewTypeAnimal].View || m_ViewTypesByType[JMESPViewTypeInfected].View)
				includeCreatures = true;
		}
		else if (m_ViewTypesByType[JMESPViewTypeBuilding].View)
		{
			flags |= QueryFlags.STATIC;
		}
		else if (CommunityOnlineToolsBase.s_HypeTrain_Loco_Type && m_ViewTypesByType[JMESPViewTypeTrain].View)
		{
			flags |= QueryFlags.STATIC;
		}

		array<Object> excluded = {};
		array<Object> collided = {};
		array<EntityAI> entities = {};

		for (int x = -numIterations; x < numIterations; x++)
		{
			for (int z = -numIterations; z < numIterations; z++)
			{
				float xx0 = (x + 0) * sizePerBox;
				float zz0 = (z + 0) * sizePerBox;

				float xx1 = (x + 1) * sizePerBox;
				float zz1 = (z + 1) * sizePerBox;

				if (includeBushes)
				{
					vector extents = Vector(sizePerBox, 2000, sizePerBox);
					collided.Clear();
					vector center = Vector(centerPosition[0] + xx0 + sizePerBox * 0.5, centerPosition[1], centerPosition[2] + zz0 + sizePerBox * 0.5);
					g_Game.IsBoxCollidingGeometry(center, vector.Zero, extents, ObjIntersectView, ObjIntersectFire, excluded, collided);

					foreach (auto obj : collided)
					{
						objects.Insert(obj);
					}

					Sleep(100);

					if (!g_COT_ThreadESP)
						return;
				}
				else
				{
					vector min = centerPosition + Vector(xx0, -1000, zz0);
					vector max = centerPosition + Vector(xx1,  1000, zz1);

					entities.Clear();
					if (includeImmovable)
					{
						DayZPlayerUtils.PhysicsGetEntitiesInBox(min, max, entities);

						if (includeCreatures)
						{
							//! PhysicsGetEntitiesInBox doesn't include creatures

							array<EntityAI> creatures = {};
							DayZPlayerUtils.SceneGetEntitiesInBox(min, max, creatures);

							foreach (auto creature : creatures)
							{
								if (creature.IsDayZCreature())
									objects.Insert(creature);
							}
						}
					}
					else
					{
						DayZPlayerUtils.SceneGetEntitiesInBox(min, max, entities, flags);
					}

					foreach (auto entity : entities)
					{
						objects.Insert(entity);
					}

					_Sleep( 1, totalTimeTaken, sleepIdx );

					if (!g_COT_ThreadESP)
						return;
				}
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

		if (!g_COT_ThreadESP && !g_COT_ThreadESP_Running)
		{
			g_COT_ThreadESP = true;
			g_Game.GameScript.Call( this, "ThreadESP", NULL );
		}
	}

	void ThreadESP()
	{
		g_COT_ThreadESP_Running = true;

		Print("+ThreadESP");

		while ( g_COT_ThreadESP )
		{
			int totalTimeTaken = 0;
			bool didRun = false;

			JMESPMeta meta;

			if ( m_StateChanged && !m_IsDestroyingWidgets && !m_IsCreatingWidgets )
			{
				m_StateChanged = false;

				didRun = true;

				set<Object> objects = new set<Object>;
				set<Object> addedObjects = new set<Object>;

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

					if ( m_ViewTypesByType[JMESPViewTypePlayer].View || m_ViewTypesByType[JMESPViewTypePlayerAI].View )
					{
						foreach ( Man playerBase : ClientData.m_PlayerBaseList )
						{
							if ( playerBase )
								objects.Insert( playerBase );
						}
					}

					if (!g_COT_ThreadESP)
						break;

					COT_String filter = Filter;
					bool requireAllKeywords;
					TStringArray keywords = filter.KeywordSearch_Prepare(requireAllKeywords);

					for ( int i = 0; i < objects.Count(); ++i )
					{
						Object obj = objects[i];

						if ( obj == NULL )
							continue;

						COT_String type = JMESPMeta.GetObjectType(obj);
						type.ToLower();

						if ( type == "#particlesourceenf" )
							continue;

						if ( !m_IknowWhatIamDoing )
						{
							if ( !IsMissionOffline() && !obj.HasNetworkID() )
								continue;

							if ( obj.IsInherited( Particle ) )
								continue;

							if ( obj.IsInherited( Camera ) )
								continue;

							//! SceneGetEntitiesInBox with QueryFlags.STATIC includes buildings without physics body
							//! (e.g. clutter cutters or Expansion dbg objs), unlike PhysicsGetEntitiesInBox,
							//! so for consistency we filter those out unless including all objects
							if ( obj.IsBuilding() && !dBodyIsSet(obj) )
								continue;
						}

						if (filter != "")
						{
							if (!type.KeywordSearchImpl(filter, keywords, requireAllKeywords))
								continue;
						}

						meta = m_MappedESPObjects.Get( obj );
						if ( meta != NULL )
						{
							#ifdef JM_COT_ESP_DEBUG
							bool metaIsValid = meta.IsValid();
							#ifdef COT_DEBUGLOGS
							Print( "-" + meta.ClassName() + "::IsValid() = " + metaIsValid.ToString() );
							#endif
							if ( metaIsValid )
							#else
							if ( meta.IsValid() )
							#endif
							{
								addedObjects.Insert( obj );
							}
						}
						else
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
								#ifdef COT_DEBUGLOGS
								Print( "-" + validViewTypes[j].ClassName() + "::IsValid( obj = " + Object.GetDebugName( obj ) + " ) = " + viewTypeIsValid.ToString() );
								#endif
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

					if (m_ViewTypesByType[JMESPViewTypeBush].View)
						Sleep(100);
					else
						_Sleep( 1, totalTimeTaken );

					if (!g_COT_ThreadESP)
						break;

					#ifdef JM_COT_ESP_DEBUG
					#ifdef COT_DEBUGLOGS
					Print( "+JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif
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
					#ifdef COT_DEBUGLOGS
					Print( "-JMESPModule::ThreadESP() - Verifying ESP Objects" );
					#endif
					#endif

					g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( CreateNewWidgets );
				}
			
				g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DestroyOldWidgets );
				
				if ( m_CurrentState == JMESPState.Update )
				{
					m_StateChanged = true;
				}
			}
			else if (m_RemoveDeleted)
			{
				for ( k = m_ActiveESPObjects.Count() - 1; k >= 0; --k )
				{
					meta = m_ActiveESPObjects[k];
					Object target = meta.target;

					//! In SP client, target will be null instantly when deleted, but in MP client,
					//! only after object has been deleted on server, so we need to check m_TargetDeleted
					if ( !target || target.ToDelete() || meta.m_TargetDeleted )
						m_ESPToDestroy.Insert( meta );
				}
			
				g_Game.GetCallQueue( CALL_CATEGORY_SYSTEM ).Call( DestroyOldWidgets );
			}

			m_RemoveDeleted = false;

			if ( didRun && m_StateChanged && m_CurrentState == JMESPState.Update )
			{				
				Sleep( Math.Max( 0, ( ESPUpdateTime * 1000 ) - totalTimeTaken ) );
			} else
			{
				Sleep( 50 );
			}
		}

		g_COT_ThreadESP_Running = false;

		Print("-ThreadESP");
	}

	override int GetRPCMin()
	{
		return JMESPModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMESPModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
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

		// Basebuilding ESP
		case JMESPModuleRPC.BaseBuilding_Build:
			RPC_BaseBuilding_Build( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_Dismantle:
			RPC_BaseBuilding_Dismantle( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_Repair:
			RPC_BaseBuilding_Repair( ctx, sender, target );
			break;
		case JMESPModuleRPC.BaseBuilding_SetPartHealth:
			RPC_BaseBuilding_SetPartHealth( ctx, sender, target );
			break;

		case JMESPModuleRPC.Vehicle_Unstuck:
			RPC_Vehicle_Unstuck( ctx, sender, target );
			break;
		case JMESPModuleRPC.Vehicle_Refuel:
			RPC_Vehicle_Refuel( ctx, sender, target );
			break;

		case JMESPModuleRPC.Heal:
			RPC_Heal( ctx, sender, target );
			break;

		case JMESPModuleRPC.MakeItemSet:
			RPC_MakeItemSet( ctx, sender, target );
			break;
		case JMESPModuleRPC.DuplicateAll:
			RPC_DuplicateAll( ctx, sender, target );
			break;
		case JMESPModuleRPC.DeleteAll:
			RPC_DeleteAll( ctx, sender, target );
			break;
		case JMESPModuleRPC.MoveToCursor:
			RPC_MoveToCursor( ctx, sender, target );
			break;

		case JMESPModuleRPC.ObjectAction:
			RPC_ObjectAction( ctx, sender, target );
			break;
		case JMESPModuleRPC.ObjectActionResult:
			RPC_ObjectActionResult( ctx, sender, target );
			break;

		case JMESPModuleRPC.UndoLastAction:
			RPC_UndoLastAction( ctx, sender, target );
			break;
		case JMESPModuleRPC.RedoLastAction:
			RPC_RedoLastAction( ctx, sender, target );
			break;

		case JMESPModuleRPC.RecordTransformHistory:
			RPC_RecordTransformHistory( ctx, sender, target );
			break;
		case JMESPModuleRPC.SetAttachment:
			RPC_SetAttachment( ctx, sender, target );
			break;
		case JMESPModuleRPC.RemoveAttachment:
			RPC_RemoveAttachment( ctx, sender, target );
			break;
		}
	}

	//! Pop and replay the most recent entry on the shared JMActionHistory
	//! stack - whatever it is (delete, heal, teleport, ...), not just an
	//! ESP-flavoured one. This module just happens to already have a working
	//! RPC pipe, so it hosts the parameterless trigger for all of them.
	void UndoLastAction()
	{
		if ( IsMissionOffline() )
		{
			JMActionHistory.Undo( NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMESPModuleRPC.UndoLastAction, true, NULL );
		}
	}

	void RedoLastAction()
	{
		if ( IsMissionOffline() )
		{
			JMActionHistory.Redo( NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMESPModuleRPC.RedoLastAction, true, NULL );
		}
	}

	//! No standalone permission check here - JMActionHistory checks each
	//! popped entry against the permission ITS OWN action required, which is
	//! the correct gate: undoing is not a new grant of capability.
	private void RPC_UndoLastAction( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMActionHistory.Undo( senderRPC );
	}

	private void RPC_RedoLastAction( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMActionHistory.Redo( senderRPC );
	}

	//! One push per move/rotate gesture (JMESPWidgetHandler.EndDrag), not per
	//! throttled SetPosition/SetOrientation RPC sent during the drag itself -
	//! those share their RPC with continuous per-frame updates and have no
	//! per-gesture boundary of their own to record an undo step at.
	void RecordTransformHistory( Object target, vector previousPosition, vector previousOrientation )
	{
		if ( !target )
			return;

		if ( IsMissionOffline() )
		{
			Exec_RecordTransformHistory( previousPosition, previousOrientation, target, NULL );
		} else
		{
			//! Untargeted with a network id in the payload - see SetPosition
			//! for why a targeted RPC is not safe here.
			int netLow;
			int netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( previousPosition );
			rpc.Write( previousOrientation );
			rpc.Send( NULL, JMESPModuleRPC.RecordTransformHistory, true, NULL );
		}
	}

	private void Exec_RecordTransformHistory( vector previousPosition, vector previousOrientation, Object target, PlayerIdentity ident )
	{
		if ( !target )
			return;

		JMActionHistory.Push( new JMTransformHistoryEntry( target, previousPosition, previousOrientation ) );
	}

	private void RPC_RecordTransformHistory( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{

		int netLow;
		int netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		vector previousPosition;
		vector previousOrientation;
		if ( !ctx.Read( previousPosition ) )
			return;
		if ( !ctx.Read( previousOrientation ) )
			return;

	#ifdef SERVER
		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		if ( !CommunityOnlineToolsBase.IsValidWorldPosition( previousPosition ) )
			return;

		if ( !CommunityOnlineToolsBase.IsFiniteVector( previousOrientation ) )
			return;

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetPosition", senderRPC, instance ) )
			return;

		Exec_RecordTransformHistory( previousPosition, previousOrientation, obj, senderRPC );
	#endif
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
			rpc.Send( NULL, JMESPModuleRPC.Log, true, NULL );
		}
	}

	private void Exec_Log( string log, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		GetCommunityOnlineToolsBase().Log( ident, "ESP: " + log );
		SendWebhookColored( "Log", instance, "Logging ESP action: " + log, JMConstants.WEBHOOK_COLOR_INFO );
	}

	//! An admin's ESP view state, written to the log and the webhook.
	//!
	//! Everything here is a guard against a client that is not the ESP form:
	//! the handler appends to the admin log file and posts a webhook, so
	//! unchecked it is a way for any player on the server to write whatever
	//! they like into the audit trail, fill the log with it, and flood the
	//! Discord endpoint from someone else's IP.
	//!
	//! The text is capped rather than rejected on length: the real sender only
	//! ever writes a few fixed phrases, so anything longer is not a message
	//! worth keeping whole.
	private void RPC_Log( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string log;
		if ( !ctx.Read( log ) )
			return;

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.View", senderRPC, instance ) )
			return;

		if ( !JMRPCThrottle.Allow( senderRPC.GetId(), "esp_log", LOG_MIN_INTERVAL_MS ) )
			return;

		if ( log.Length() > LOG_MAX_LENGTH )
			log = log.Substring( 0, LOG_MAX_LENGTH );

		Exec_Log( log, senderRPC, instance );
	}

	//! Sent UNTARGETED, with the target carried as a network id in the payload
	//! rather than as the RPC's target object.
	//!
	//! A targeted ScriptRPC (rpc.Send(target, ...)) killed the server outright:
	//! vanilla DayZGame.OnRPC dispatches a targeted RPC into native handling and
	//! then on to target.OnRPC() BEFORE any of this module's script ever runs,
	//! and with one of these ids aimed at a vehicle that path died in a native
	//! memmove writing off the end of the thread stack (0xC0000005, identical
	//! fault address on every crash). Instrumentation proved it: this module's
	//! OnRPC was never entered for the targeted transform RPCs, while the
	//! untargeted Log RPC on the same switch was handled normally every time.
	//!
	//! Sending untargeted and resolving the object server-side via
	//! GetObjectByNetworkId keeps the whole exchange in script, and matches
	//! what the rest of the mod already does (JMPlayerModule's spectate RPC,
	//! JMVehiclesModule throughout, and RPC_DeleteObject just below).
	void SetPosition( vector position, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetPosition( position, target, NULL );
		} else
		{
			if ( !target )
				return;

			int netLow;
			int netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( position );
			rpc.Send( NULL, JMESPModuleRPC.SetPosition, true, NULL );
		}
	}

	private void Exec_SetPosition( vector position, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		Transport transport;
		if ( Class.CastTo( transport, target ) )
		{
			CommunityOnlineToolsBase.ForceTransportPositionAndOrientation(transport, position, transport.GetOrientation());
		}
		else
		{
			target.SetPosition( position );

		#ifdef SERVER
			//! Need to update position for dead players on clients via RPC
			if (target.IsMan() && !target.IsAlive())
				SetPosition(position, target);
		#endif
		}

		if ( !ShouldRecordMove( ident, "esp_move" ) )
			return;

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=position value=" + position );
		SendWebhookColored( "Position", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") position to " + position.ToString(), JMConstants.WEBHOOK_COLOR_ESP );
	}

	//! Whether this admin's continuous transform should be written down again.
	//!
	//! An offline host has no identity and is never throttled - there is no
	//! webhook and no remote sender to spend anything.
	protected bool ShouldRecordMove( PlayerIdentity ident, string bucket )
	{
		if ( !ident )
			return true;

		return JMRPCThrottle.Allow( ident.GetId(), bucket, MOVE_RECORD_INTERVAL_MS );
	}

	private void RPC_SetPosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{

		int netLow;
		int netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		vector position;
		if ( !ctx.Read( position ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

	#ifdef SERVER
		if ( !CommunityOnlineToolsBase.IsValidWorldPosition( position ) )
			return;

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetPosition", senderRPC, instance ) )
			return;

		Exec_SetPosition( position, obj, senderRPC, instance );
	#else
		obj.SetPosition( position );
	#endif
	}

	//! Untargeted with a network id in the payload - see SetPosition above for
	//! why a targeted RPC is not safe here.
	void SetOrientation( vector orientation, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetOrientation( orientation, target, NULL );
		} else
		{
			if ( !target )
				return;

			int netLow;
			int netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( orientation );
			rpc.Send( NULL, JMESPModuleRPC.SetOrientation, true, NULL );
		}
	}

	private void Exec_SetOrientation( vector orientation, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;


		Transport transport;
		if ( Class.CastTo( transport, target ) )
		{
			CommunityOnlineToolsBase.ForceTransportPositionAndOrientation(transport, transport.GetPosition(), orientation);
		}
		else
		{
			target.SetOrientation( orientation );

		#ifdef SERVER
			//! Need to update orientation for dead players on clients via RPC
			if (target.IsMan() && !target.IsAlive())
				SetOrientation(orientation, target);
		#endif
		}

		if ( !ShouldRecordMove( ident, "esp_turn" ) )
			return;

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=orientation value=" + orientation );
		SendWebhookColored( "Orientation", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") orientation to " + orientation.ToString(), JMConstants.WEBHOOK_COLOR_ESP );
	}

	private void RPC_SetOrientation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{

		int netLow;
		int netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		vector orientation;
		if ( !ctx.Read( orientation ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

	#ifdef SERVER
		if ( !CommunityOnlineToolsBase.IsFiniteVector( orientation ) )
			return;

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetOrientation", senderRPC ) )
			return;

		Exec_SetOrientation( orientation, obj, senderRPC, instance );
	#else
		obj.SetOrientation( orientation );
	#endif
	}

	void SetHealth( float health, string zone, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetHealth( health, zone, target, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - vanilla OnRPC handles
			//! that natively before any script runs and a stale/mismatched target
			//! crashes the server outright (docs/systems/rpc.md - "Never send a
			//! targeted RPC"). Send untargeted, carry the network id instead.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( health );
			rpc.Send( NULL, JMESPModuleRPC.SetHealth, true, NULL );
		}
	}

	private void Exec_SetHealth( float health, string zone, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		if ( (target.IsInherited(Man) || target.IsInherited(DayZCreature)) && !target.IsAlive() )
			return;

		//! The number came off the wire. Clamped to what this type of entity
		//! can actually hold, so a hand-written packet cannot hand the damage
		//! system a negative, a NaN or a value no health bar can represent.
		if ( !CommunityOnlineToolsBase.IsFiniteFloat( health ) )
			return;

		float maxHealth = MiscGameplayFunctions.GetTypeMaxGlobalHealth( target.GetType() );

		if ( maxHealth > 0 )
			health = Math.Clamp( health, 0, maxHealth );
		else
			health = Math.Max( health, 0 );

		target.SetHealth( health );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=health value=" + health );
		SendWebhookColored( "Health", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") health to " + health, JMConstants.WEBHOOK_COLOR_ESP );
	}

	private void RPC_SetHealth( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		float health;
		if ( !ctx.Read( health ) )
			return;

		string zone;
		//! TODO: Setting zone is not implemented in GUI, always sets global health
		//if ( !ctx.Read( zone ) )
			//return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetHealth", senderRPC, instance ) )
			return;

		Exec_SetHealth( health, zone, obj, senderRPC, instance );
	}

	// =========================================================================
	//  Object actions
	//
	//  One RPC for every per-object property the panel can set. They share a
	//  channel because they share a shape - an action id, an int and a float -
	//  and a separate RPC each would be a dozen near-identical triples of
	//  client call, exec and handler for no gain. The permission is still per
	//  action, resolved server-side from the id.
	// =========================================================================

	//! Attach `className` in `slotId`, replacing whatever is already there.
	//! Not routed through ObjectAction - see RPC.c's note on why this pair
	//! gets its own RPC ids instead.
	void SetAttachment( Object target, int slotId, string className )
	{
		if ( IsMissionOffline() )
		{
			Exec_SetAttachment( slotId, className, target, NULL );
		} else
		{
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( slotId );
			rpc.Write( className );
			rpc.Send( NULL, JMESPModuleRPC.SetAttachment, true, NULL );
		}
	}

	void RemoveAttachment( Object target, int slotId )
	{
		if ( IsMissionOffline() )
		{
			Exec_RemoveAttachment( slotId, target, NULL );
		} else
		{
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( slotId );
			rpc.Send( NULL, JMESPModuleRPC.RemoveAttachment, true, NULL );
		}
	}

	private void RPC_SetAttachment( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh, slotId;
		string className;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) || !ctx.Read( slotId ) || !ctx.Read( className ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetAttachment", senderRPC, instance ) )
			return;

		Exec_SetAttachment( slotId, className, obj, senderRPC, instance );
	}

	private void RPC_RemoveAttachment( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh, slotId;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) || !ctx.Read( slotId ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetAttachment", senderRPC, instance ) )
			return;

		Exec_RemoveAttachment( slotId, obj, senderRPC, instance );
	}

	private void Exec_SetAttachment( int slotId, string className, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		EntityAI entity = EntityAI.Cast( target );
		if ( !entity || !entity.GetInventory() )
			return;

		GameInventory inventory = entity.GetInventory();

		EntityAI existing = inventory.FindAttachment( slotId );
		if ( existing )
			g_Game.ObjectDelete( existing );

		inventory.CreateAttachmentEx( className, slotId );

		GetCommunityOnlineToolsBase().Log( ident, "Set attachment " + className + " on " + entity.GetType() );
	}

	private void Exec_RemoveAttachment( int slotId, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		EntityAI entity = EntityAI.Cast( target );
		if ( !entity || !entity.GetInventory() )
			return;

		EntityAI existing = entity.GetInventory().FindAttachment( slotId );
		if ( !existing )
			return;

		GetCommunityOnlineToolsBase().Log( ident, "Removed attachment " + existing.GetType() + " from " + entity.GetType() );

		g_Game.ObjectDelete( existing );
	}

	void ObjectAction( int action, int ivalue, float fvalue, Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_ObjectAction( action, ivalue, fvalue, target, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( action );
			rpc.Write( ivalue );
			rpc.Write( fvalue );
			rpc.Send( NULL, JMESPModuleRPC.ObjectAction, true, NULL );
		}
	}

	//! The permission an action needs. An action with no entry here is not
	//! dispatchable at all - a new id cannot reach an object by being forgotten
	//! about.
	static string PermissionForAction( int action )
	{
		switch ( action )
		{
			case JMESPObjectAction.SetQuantity:
				return "ESP.Object.SetQuantity";
			case JMESPObjectAction.SetLiquid:
				return "ESP.Object.SetLiquid";
			case JMESPObjectAction.SetCleanness:
				return "ESP.Object.SetCleanness";
			case JMESPObjectAction.SetFoodStage:
				return "ESP.Object.SetFoodStage";
			case JMESPObjectAction.SetOpen:
				return "ESP.Object.OpenClose";
			case JMESPObjectAction.SetLock:
				return "ESP.Object.Lock";
			case JMESPObjectAction.SetCode:
				return "ESP.Object.SetCode";
			case JMESPObjectAction.GetCode:
				return "ESP.Object.GetCode";

			case JMESPObjectAction.SetJammed:
			case JMESPObjectAction.SetChambered:
			case JMESPObjectAction.LoadMagazine:
				return "ESP.Object.Weapon";

			case JMESPObjectAction.SetBurning:
			case JMESPObjectAction.FireplaceRefuel:
			case JMESPObjectAction.SetStoneCircle:
			case JMESPObjectAction.SetOven:
			case JMESPObjectAction.CookAll:
				return "ESP.Object.Fireplace";

			//! The same permission as locking one, because that is what it is -
			//! the radius only changes how many at a time.
			case JMESPObjectAction.SetLockAll:
				return "ESP.Object.Lock";

			case JMESPObjectAction.SetLockWheels:
				return "ESP.Object.Car.LockWheels";

			case JMESPObjectAction.RefillCoolant:
				return "ESP.Object.Car.Refuel";

			case JMESPObjectAction.RepairAndFillSlots:
				return "ESP.Object.Heal";

			case JMESPObjectAction.ChangeColor:
				return "ESP.Object.ChangeColor";

			case JMESPObjectAction.SetGrenadePin:
				return "ESP.Object.SetQuantity";

			//! Traps have their own permission now that arming is offered
			//! beside triggering. It is a child of ESP.Object like the rest, so
			//! a role that allows the parent keeps working unchanged.
			case JMESPObjectAction.TriggerTrap:
			case JMESPObjectAction.SetTrapArmed:
				return "ESP.Object.Trap";

			case JMESPObjectAction.SetFenceOpen:
				return "ESP.Object.BaseBuilding.Build";

			case JMESPObjectAction.SetTentOpen:
			case JMESPObjectAction.SetCarDoors:
			case JMESPObjectAction.SetHouseDoorOpen:
			case JMESPObjectAction.SetHouseDoorLocked:
				return "ESP.Object.OpenClose";

			//! Setting a fluid to a fraction is the same power as filling it.
			case JMESPObjectAction.SetFuel:
			case JMESPObjectAction.SetCoolant:
				return "ESP.Object.Car.Refuel";

			case JMESPObjectAction.FillInternalMagazine:
				return "ESP.Object.Weapon";

			case JMESPObjectAction.BuildAll:
				return "ESP.Object.BaseBuilding.Build";

			case JMESPObjectAction.DismantleAll:
				return "ESP.Object.BaseBuilding.Dismantle";

			case JMESPObjectAction.RepairAll:
				return "ESP.Object.BaseBuilding.Repair";

			case JMESPObjectAction.SetFlagRaised:
				return "ESP.Object.Flag";

			case JMESPObjectAction.ClearCargo:
				return "ESP.Object.ClearCargo";

			case JMESPObjectAction.SetImmobilized:
				return "ESP.Object.Immobilize";
		}

		return "";
	}

	//! The lock that governs this object: the object itself if it is one, or
	//! the one attached to it if it is a gate.
	static CombinationLock GetCombinationLock( Object target )
	{
		CombinationLock combo;

		if ( Class.CastTo( combo, target ) )
			return combo;

		EntityAI entity = EntityAI.Cast( target );

		if ( !entity || !entity.GetInventory() )
			return NULL;

		for ( int i = 0; i < entity.GetInventory().AttachmentCount(); ++i )
		{
			if ( Class.CastTo( combo, entity.GetInventory().GetAttachmentFromIndex( i ) ) )
				return combo;
		}

		return NULL;
	}

	private void Exec_ObjectAction( int action, int ivalue, float fvalue, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		ItemBase item = ItemBase.Cast( target );

		string logValue = ivalue.ToString();

		switch ( action )
		{
			case JMESPObjectAction.SetQuantity:
				if ( !item )
					return;
				if ( !CommunityOnlineToolsBase.IsFiniteFloat( fvalue ) )
					return;
				//! Same reasoning as SetHealth: the value is a client's, and
				//! an item's own maximum is the only bound that means anything.
				fvalue = Math.Clamp( fvalue, 0, item.GetQuantityMax() );
				item.SetQuantity( fvalue, false );
				logValue = fvalue.ToString();
				break;

			case JMESPObjectAction.SetLiquid:
				if ( !item )
					return;
				//! Liquid types are single bits by engine rule - "0 or any
				//! power of two" - so a negative, or a value with more than one
				//! bit set, is not a liquid the game defines.
				if ( ivalue < 0 || ( ivalue & ( ivalue - 1 ) ) != 0 )
					return;
				item.SetLiquidType( ivalue );
				break;

			case JMESPObjectAction.SetCleanness:
				if ( !item )
					return;
				//! Cleanness is a flag, not a scale.
				if ( ivalue < 0 || ivalue > 1 )
					return;
				item.SetCleanness( ivalue );
				break;

			case JMESPObjectAction.SetFoodStage:
				Edible_Base edible = Edible_Base.Cast( target );
				if ( !edible )
					return;
				//! A stage outside the enum is not a food state the item has a
				//! transition for.
				if ( ivalue < FoodStageType.NONE || ivalue >= FoodStageType.COUNT )
					return;
				edible.ChangeFoodStage( ivalue );
				break;

			case JMESPObjectAction.SetOpen:
				if ( !item )
					return;
				if ( ivalue )
					item.Open();
				else
					item.Close();
				break;

			case JMESPObjectAction.SetLock:
				Exec_SetLock( target, ivalue );
				break;

			case JMESPObjectAction.SetCode:
				Exec_SetCode( target, ivalue );
				break;

			case JMESPObjectAction.GetCode:
				Exec_GetCode( target, ident );
				return;

			case JMESPObjectAction.SetJammed:
				if ( !Exec_SetJammed( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.SetChambered:
				if ( !Exec_SetChambered( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.LoadMagazine:
				if ( !Exec_LoadMagazine( target ) )
					return;
				break;

			case JMESPObjectAction.SetBurning:
				if ( !Exec_SetBurning( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.FireplaceRefuel:
				if ( !Exec_FireplaceRefuel( target ) )
					return;
				break;

			case JMESPObjectAction.SetStoneCircle:
				if ( !Exec_SetStoneCircle( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.SetOven:
				if ( !Exec_SetOven( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.CookAll:
				logValue = Exec_CookAll( target ).ToString();
				break;

			case JMESPObjectAction.SetLockAll:
				logValue = Exec_SetLockAll( target, ivalue ).ToString();
				break;

			case JMESPObjectAction.SetLockWheels:
				Exec_SetLockWheels( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.RefillCoolant:
				Exec_RefillCoolant( target, ident, instance );
				return;

			case JMESPObjectAction.RepairAndFillSlots:
				Exec_RepairAndFillSlots( target, ident, instance );
				return;

			case JMESPObjectAction.ChangeColor:
				Exec_ChangeColor( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.SetGrenadePin:
				Exec_SetGrenadePin( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.TriggerTrap:
				Exec_TriggerTrap( target, ident, instance );
				return;

			case JMESPObjectAction.SetFenceOpen:
				Exec_SetFenceOpen( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.SetTentOpen:
				Exec_SetTentOpen( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.SetCarDoors:
				Exec_SetCarDoors( target, ivalue, ident, instance );
				return;

			case JMESPObjectAction.SetHouseDoorOpen:
				Exec_SetHouseDoorOpen( target, ivalue, fvalue, ident, instance );
				return;

			case JMESPObjectAction.SetHouseDoorLocked:
				Exec_SetHouseDoorLocked( target, ivalue, fvalue, ident, instance );
				return;

			case JMESPObjectAction.SetFuel:
				CommunityOnlineToolsBase.SetFuel01( target, fvalue );
				logValue = fvalue.ToString();
				break;

			case JMESPObjectAction.SetCoolant:
				CommunityOnlineToolsBase.SetCoolant01( target, fvalue );
				logValue = fvalue.ToString();
				break;

			case JMESPObjectAction.FillInternalMagazine:
				if ( !Exec_FillInternalMagazine( target ) )
					return;
				break;

			case JMESPObjectAction.BuildAll:
				logValue = Exec_BuildAll( target, ident, instance ).ToString();
				break;

			case JMESPObjectAction.DismantleAll:
				logValue = Exec_DismantleAll( target, ident, instance ).ToString();
				break;

			case JMESPObjectAction.RepairAll:
				logValue = Exec_RepairAll( target, ident, instance ).ToString();
				break;

			case JMESPObjectAction.SetFlagRaised:
				if ( !Exec_SetFlagRaised( target, fvalue ) )
					return;
				logValue = fvalue.ToString();
				break;

			case JMESPObjectAction.ClearCargo:
				logValue = CommunityOnlineToolsBase.ClearCargo( EntityAI.Cast( target ) ).ToString();
				break;

			case JMESPObjectAction.SetImmobilized:
				if ( !Exec_SetImmobilized( target, ivalue ) )
					return;
				break;

			case JMESPObjectAction.SetTrapArmed:
				if ( !Exec_SetTrapArmed( target, ivalue, ident ) )
					return;
				break;

			default:
				return;
		}

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=" + action + " value=" + logValue );
		SendWebhookColored( "Object", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") action " + action + " to " + logValue, JMConstants.WEBHOOK_COLOR_ESP );
	}

	private void Exec_SetLockWheels( Object target, int ivalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		CommunityOnlineToolsBase.SetLockWheels( target, ivalue != 0 );
		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=lockwheels state=" + ivalue );
		SendWebhookColored( "SetLockWheels", instance, "Wheels lock state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_RefillCoolant( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		CommunityOnlineToolsBase.RefillCoolant( target );
		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=refillcoolant" );
		SendWebhookColored( "RefillCoolant", instance, "Refilled coolant for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_RepairAndFillSlots( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		EntityAI entity = EntityAI.Cast( target );
		if ( !entity )
			return;

		CommunityOnlineToolsBase.RepairEntityRecursive( entity );
		CommunityOnlineToolsBase.Refuel( entity );
		GetCommunityOnlineToolsBase().SpawnCompatibleAttachmentsWithColor( entity, NULL, 2, "" );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=repairandfill" );
		SendWebhookColored( "RepairAndFillSlots", instance, "Repaired and filled slots for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_ChangeColor( Object target, int colorIndex, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		EntityAI entity = EntityAI.Cast( target );
		if ( !entity )
			return;

		// Color is a whole-object property - paint the thing a sub-part
		// belongs to, never the sub-part the menu happened to be opened on.
		if ( entity.GetHierarchyRoot() )
			entity = EntityAI.Cast( entity.GetHierarchyRoot() );

		string color = JMObjectSpawnerModule.GetColorTokenAt( colorIndex );
		if ( color == "" )
			return;

		string label = target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition();

		GetCommunityOnlineToolsBase().RecolorEntityAndAttachments( entity, NULL, color, 2 );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=changecolor color=" + color );
		SendWebhookColored( "ChangeColor", instance, "Changed color to " + color + " for " + label, JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetGrenadePin( Object target, int ivalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		Grenade_Base grenade = Grenade_Base.Cast( target );
		if ( !grenade )
			return;

		if ( ivalue != 0 )
			grenade.Pin();
		else
			grenade.Unpin();

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=grenadepin state=" + ivalue );
		SendWebhookColored( "SetGrenadePin", instance, "Grenade pin state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_TriggerTrap( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		TrapBase trap = TrapBase.Cast( target );
		if ( !trap )
			return;

		trap.OnSteppedOn( NULL );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=triggertrap" );
		SendWebhookColored( "TriggerTrap", instance, "Triggered trap " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetFenceOpen( Object target, int ivalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		Fence fence = Fence.Cast( target );
		if ( !fence )
			return;

		if ( ivalue != 0 )
			fence.OpenFence();
		else
			fence.CloseFence();

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=fenceopen state=" + ivalue );
		SendWebhookColored( "SetFenceOpen", instance, "Fence open state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetHouseDoorOpen( Object target, int ivalue, float fvalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		Building house = Building.Cast( target );
		if ( !house )
			return;

		int doorIndex = fvalue;

		if ( ivalue != 0 )
			house.OpenDoor( doorIndex );
		else
			house.CloseDoor( doorIndex );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=housedooropen door=" + doorIndex + " state=" + ivalue );
		SendWebhookColored( "SetHouseDoorOpen", instance, "Door " + doorIndex + " open state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetHouseDoorLocked( Object target, int ivalue, float fvalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		Building house = Building.Cast( target );
		if ( !house )
			return;

		int doorIndex = fvalue;

		if ( ivalue != 0 )
			house.LockDoor( doorIndex );
		else
			house.UnlockDoor( doorIndex );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=housedoorlocked door=" + doorIndex + " state=" + ivalue );
		SendWebhookColored( "SetHouseDoorLocked", instance, "Door " + doorIndex + " locked state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetTentOpen( Object target, int ivalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		TentBase tent = TentBase.Cast( target );
		if ( !tent )
			return;

		if ( ivalue != 0 )
			tent.Open();
		else
			tent.Close();

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=tentopen state=" + ivalue );
		SendWebhookColored( "SetTentOpen", instance, "Tent open state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void Exec_SetCarDoors( Object target, int ivalue, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		CarScript car = CarScript.Cast( target );
		if ( !car )
			return;

		car.COT_SetCarDoors( ivalue != 0 );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=cardoors state=" + ivalue );
		SendWebhookColored( "SetCarDoors", instance, "Car doors open state set to " + ivalue + " for " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	//! Load the magazine a weapon carries INSIDE itself.
	//!
	//! Separate from LoadMagazine, which attaches one: a gun is one or the
	//! other, and the two rows never both apply to the same weapon.
	private bool Exec_FillInternalMagazine( Object target )
	{
		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, target ) )
			return false;

		if ( !weapon.HasInternalMagazine( -1 ) )
			return false;

		return weapon.FillInnerMagazine( "", WeaponWithAmmoFlags.CHAMBER );
	}

	//! Build every part that can be built right now.
	//!
	//! Only the parts the construction itself reports as buildable are passed
	//! in: COT_BuildParts walks each one's required parts on its own, so a
	//! wall's base is built by asking for the wall rather than by ordering the
	//! list here.
	private int Exec_BuildAll( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		BaseBuildingBase building;

		if ( !Class.CastTo( building, target ) || !building.GetConstruction() )
			return 0;

		bool requireMaterials = true;
		if ( !IsMissionOffline() )
			requireMaterials = !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Build.MaterialsNotRequired", ident, instance );

		map< string, ref JMConstructionPartData > parts = new map< string, ref JMConstructionPartData >;
		building.GetConstruction().COT_GetParts( parts, requireMaterials );

		TStringArray names = new TStringArray;

		for ( int i = 0; i < parts.Count(); ++i )
		{
			if ( parts.GetElement( i ).m_State == JMConstructionPartState.CAN_BUILD )
				names.Insert( parts.GetKey( i ) );
		}

		if ( names.Count() == 0 )
			return 0;

		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

		building.GetConstruction().COT_BuildParts( names, player, requireMaterials );

		SendWebhookColored( "BB_Build", instance, "Built every buildable part (" + names.Count() + ") of \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_ESP );

		return names.Count();
	}

	//! Take the whole construction apart, base part included - which is what
	//! dismantling a base building object down to nothing means, so the entity
	//! itself goes with the last part.
	private int Exec_DismantleAll( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		BaseBuildingBase building;

		if ( !Class.CastTo( building, target ) || !building.GetConstruction() )
			return 0;

		map< string, ref JMConstructionPartData > parts = new map< string, ref JMConstructionPartData >;
		building.GetConstruction().COT_GetParts( parts, false );

		TStringArray names = new TStringArray;

		for ( int i = 0; i < parts.Count(); ++i )
		{
			if ( parts.GetElement( i ).m_State == JMConstructionPartState.BUILT )
				names.Insert( parts.GetKey( i ) );
		}

		if ( names.Count() == 0 )
			return 0;

		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

		building.GetConstruction().COT_DismantleParts( names, player );

		SendWebhookColored( "BB_Dismantle", instance, "Dismantled every built part (" + names.Count() + ") of \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_WARNING );

		return names.Count();
	}

	//! Heal every already-built construction part of a base building object.
	private int Exec_RepairAll( Object target, PlayerIdentity ident = NULL, JMPlayerInstance instance = NULL )
	{
		BaseBuildingBase building;

		if ( !Class.CastTo( building, target ) || !building.GetConstruction() )
			return 0;

		map< string, ref JMConstructionPartData > parts = new map< string, ref JMConstructionPartData >;
		building.GetConstruction().COT_GetParts( parts, false );

		TStringArray names = new TStringArray;

		for ( int i = 0; i < parts.Count(); ++i )
		{
			if ( parts.GetElement( i ).m_State == JMConstructionPartState.BUILT )
				names.Insert( parts.GetKey( i ) );
		}

		if ( names.Count() == 0 )
			return 0;

		building.GetConstruction().COT_RepairParts( names );

		SendWebhookColored( "BB_Repair", instance, "Repaired every built part (" + names.Count() + ") of \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_SUCCESS );

		return names.Count();
	}

	//! Move a flag pole's mast to a fraction raised.
	//!
	//! The mast animation runs the other way round - 0 is fully raised - and
	//! the refresher timer is moved with it, because in vanilla a flag's height
	//! IS the remaining refresh time and leaving the two apart makes a raised
	//! flag that refreshes nothing.
	private bool Exec_SetFlagRaised( Object target, float raised01 )
	{
		TerritoryFlag flag;

		if ( !Class.CastTo( flag, target ) )
			return false;

		float raised = Math.Clamp( raised01, 0, 1 );

		flag.AnimateFlag( 1 - raised );
		flag.AddRefresherTime01( raised - flag.GetRefresherTime01() );

		return true;
	}

	private bool Exec_SetImmobilized( Object target, int state )
	{
		ZombieBase zombie;
		AnimalBase animal;

		if ( Class.CastTo( zombie, target ) )
			zombie.COT_SetImmobilized( state != 0 );
		else if ( Class.CastTo( animal, target ) )
			animal.COT_SetImmobilized( state != 0 );
		else
			return false;

		return true;
	}

	//! Arm or disarm a trap where it lies.
	//!
	//! Arming goes through StartActivate rather than SetActive so it takes the
	//! trap's own arming delay and, through the COT override, records the admin
	//! who armed it as the owner of whatever it later kills.
	private bool Exec_SetTrapArmed( Object target, int armed, PlayerIdentity ident = NULL )
	{
		TrapBase trap;

		if ( !Class.CastTo( trap, target ) )
			return false;

		if ( armed != 0 )
		{
			if ( trap.IsActive() )
				return false;

			PlayerBase player;
			Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

			trap.StartActivate( player );

			return true;
		}

		trap.SetInactive();

		return true;
	}

	// =========================================================================
	//  Weapons
	// =========================================================================

	//! Jam or clear a jam.
	//!
	//! Unjamming re-rolls the weapon's state machine the same way the player
	//! manager's inventory unjam does - a weapon left in its jammed FSM state
	//! with the flag cleared answers every later action as if it were still
	//! stuck.
	private bool Exec_SetJammed( Object target, int jammed )
	{
		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, target ) )
			return false;

		if ( weapon.IsJammed() == ( jammed != 0 ) )
			return false;

		weapon.SetJammed( jammed != 0 );
		weapon.RandomizeFSMState();
		weapon.Synchronize();

		return true;
	}

	//! Put a round in the chamber or take one out.
	//!
	//! The cartridge to push is whatever the weapon itself says it can take -
	//! GetRandomChamberableAmmoTypeName answers per muzzle, so a chambering
	//! action never has to know what calibre it is looking at.
	private bool Exec_SetChambered( Object target, int chambered )
	{
		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, target ) )
			return false;

		int muzzle = weapon.GetCurrentMuzzle();
		bool changed;

		if ( chambered == 0 )
		{
			if ( weapon.IsChamberEmpty( muzzle ) )
				return false;

			float popDamage;
			string popType;

			changed = weapon.PopCartridgeFromChamber( muzzle, popDamage, popType );
		}
		else
		{
			if ( weapon.IsChamberFull( muzzle ) )
				return false;

			string ammoType = weapon.GetRandomChamberableAmmoTypeName( muzzle );

			if ( ammoType == "" )
				return false;

			changed = weapon.PushCartridgeToChamber( muzzle, 0, ammoType );
		}

		//! Push/PopCartridgeFromChamber only touch the chamber data - unlike
		//! every other FSM-relevant change in this file, they don't drive the
		//! state machine themselves. Vanilla always follows a chamber change
		//! with this same pair ("FSM cares about chamber state", see
		//! Weapon_Base.OnStoreSave-adjacent call sites in weapon_base.c) -
		//! without it the chamber data changes on the server but the FSM/anim
		//! state never catches up, so the action looks like it did nothing.
		if ( changed )
		{
			weapon.RandomizeFSMState();
			weapon.Synchronize();
		}

		return changed;
	}

	//! Attach a full magazine of whatever the weapon is configured to take.
	//!
	//! The type comes from the weapon's own magazines[] array rather than from
	//! a list here: every weapon declares what fits it, and a table in this file
	//! would be one more thing to keep in step with every mod that adds a gun.
	private bool Exec_LoadMagazine( Object target )
	{
		Weapon_Base weapon;

		if ( !Class.CastTo( weapon, target ) )
			return false;

		TStringArray magazines = new TStringArray;
		g_Game.ConfigGetTextArray( "CfgWeapons " + weapon.GetType() + " magazines", magazines );

		if ( magazines.Count() == 0 )
			g_Game.ConfigGetTextArray( "CfgVehicles " + weapon.GetType() + " magazines", magazines );

		if ( magazines.Count() == 0 )
			return false;

		//! Already wearing one: swap rather than stack a second in a slot that
		//! only holds one anyway.
		Magazine existing;

		for ( int i = 0; i < weapon.GetInventory().AttachmentCount(); i++ )
		{
			if ( Class.CastTo( existing, weapon.GetInventory().GetAttachmentFromIndex( i ) ) )
			{
				existing.ServerSetAmmoMax();
				return true;
			}
		}

		Magazine created;

		if ( !Class.CastTo( created, weapon.GetInventory().CreateAttachment( magazines[0] ) ) )
			return false;

		created.ServerSetAmmoMax();

		return true;
	}

	// =========================================================================
	//  Fireplaces
	// =========================================================================

	//! Light or put out a fire.
	//!
	//! force_start is passed so a fireplace that thinks it is already burning
	//! still re-runs its start path - an admin pressing ignite on something
	//! that looks unlit means "be lit", not "toggle a flag".
	//!
	//! A fire with nothing to burn goes straight back out on its first heating
	//! tick, which is why refuel exists beside this.
	private bool Exec_SetBurning( Object target, int burning )
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, target ) )
			return false;

		if ( burning != 0 )
			fireplace.StartFire( true );
		else
			fireplace.StopFire();

		return true;
	}

	//! Fill the fireplace with firewood.
	//!
	//! Created in its cargo rather than handed to the admin: the point is a fire
	//! that will keep burning, and fuel in an inventory somewhere else does not
	//! do that.
	private bool Exec_FireplaceRefuel( Object target )
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, target ) || !fireplace.GetInventory() )
			return false;

		ItemBase wood;

		if ( !Class.CastTo( wood, fireplace.GetInventory().CreateInInventory( FUEL_TYPE ) ) )
			return false;

		wood.SetQuantityMax();

		return true;
	}

	private bool Exec_SetStoneCircle( Object target, int state )
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, target ) )
			return false;

		fireplace.SetStoneCircleState( state != 0 );

		return true;
	}

	private bool Exec_SetOven( Object target, int state )
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, target ) )
			return false;

		fireplace.SetOvenState( state != 0 );

		return true;
	}

	//! Finish every piece of food on the fireplace, and in anything standing on
	//! it - a pot or a pan is an attachment holding its own cargo.
	//!
	//! Answers how many items it changed, which is what the log line wants: a
	//! cook-all over an empty fireplace and one over a full pot are the same
	//! action and should not read the same afterwards.
	private int Exec_CookAll( Object target )
	{
		FireplaceBase fireplace;

		if ( !Class.CastTo( fireplace, target ) || !fireplace.GetInventory() )
			return 0;

		int cooked = CookContainer( fireplace );

		for ( int i = 0; i < fireplace.GetInventory().AttachmentCount(); i++ )
		{
			EntityAI attachment = fireplace.GetInventory().GetAttachmentFromIndex( i );

			if ( attachment )
				cooked += CookContainer( attachment );
		}

		return cooked;
	}

	//! Cook everything directly in `container`'s cargo. Not recursive past one
	//! level on purpose: a fireplace holds pots, a pot holds food, and nothing
	//! sensible is nested deeper than that.
	private int CookContainer( EntityAI container )
	{
		//! An attachment need not have an inventory at all - a fireplace tripod
		//! holds one, a stone does not.
		if ( !container || !container.GetInventory() )
			return 0;

		CargoBase cargo = container.GetInventory().GetCargo();

		if ( !cargo )
			return 0;

		int cooked = 0;

		for ( int i = 0; i < cargo.GetItemCount(); i++ )
		{
			Edible_Base edible;

			if ( !Class.CastTo( edible, cargo.GetItem( i ) ) )
				continue;

			int stage = BestCookedStage( edible.GetType() );

			if ( stage == FoodStageType.NONE )
				continue;

			edible.ChangeFoodStage( stage );
			cooked++;
		}

		return cooked;
	}

	//! The stage this class calls "cooked".
	//!
	//! Read off the config rather than assumed to be BAKED: there is no script
	//! query for which stages a class declares, and asking for one it does not
	//! have spawns it unchanged - the action would look like it did nothing.
	private int BestCookedStage( string classname )
	{
		TStringArray names  = { "Baked", "Boiled", "Dried" };
		TIntArray    stages = { FoodStageType.BAKED, FoodStageType.BOILED, FoodStageType.DRIED };

		for ( int i = 0; i < names.Count(); i++ )
		{
			if ( g_Game.ConfigIsExisting( "CfgVehicles " + classname + " Food FoodStages " + names[i] ) )
				return stages[i];
		}

		return FoodStageType.NONE;
	}

	// =========================================================================
	//  Locks
	// =========================================================================

	//! Lock or unlock every combination lock standing near the target.
	//!
	//! A base is not one entity - every gate, door and tower carries its own
	//! lock - so "lock the base" cannot be addressed to a single object. The
	//! radius is what makes it one action instead of one right-click per gate.
	//!
	//! Answers how many locks it touched so the log says what actually happened
	//! rather than just that the button was pressed.
	private int Exec_SetLockAll( Object target, int locked )
	{
		array<Object> nearby = new array<Object>;
		array<CargoBase> proxy = new array<CargoBase>;

		g_Game.GetObjectsAtPosition3D( target.GetPosition(), LOCK_ALL_RADIUS, nearby, proxy );

		int touched = 0;

		for ( int i = 0; i < nearby.Count(); i++ )
		{
			CombinationLock combo = GetCombinationLock( nearby[i] );

			if ( !combo )
				continue;

			//! An object and the lock hanging off it both answer with the same
			//! lock, so the same one arrives twice.
			if ( combo.IsLocked() == ( locked != 0 ) )
				continue;

			Exec_SetLock( nearby[i], locked );
			touched++;
		}

		return touched;
	}

	//! Locking runs the vanilla server path so the slot lock and the shuffle
	//! happen too. Unlocking does NOT: UnlockServer is the "take the lock off"
	//! action and drops it on the ground. An admin unlocking a gate wants it
	//! open, not disassembled, so the dial is simply turned back to the locked
	//! combination - which is exactly what the lock itself calls unlocked.
	private void Exec_SetLock( Object target, int locked )
	{
		CombinationLock combo = GetCombinationLock( target );

		if ( !combo )
			return;

		if ( locked )
		{
			combo.LockServer( EntityAI.Cast( combo.GetHierarchyParent() ) );
			return;
		}

		combo.SetCombination( combo.m_CombinationLocked );
		combo.CheckLockedStateServer();
		combo.Synchronize();
	}

	//! Changing the code keeps the lock in the state it was already in: a
	//! locked gate stays locked on the new code rather than falling open.
	private void Exec_SetCode( Object target, int code )
	{
		CombinationLock combo = GetCombinationLock( target );

		if ( !combo )
			return;

		//! The lock net-syncs its combination as an int registered over exactly
		//! the range its dials can show. A code from outside that range - which
		//! is anything a hand-written packet cares to send - is a value the
		//! sync cannot carry, and leaves a lock nobody can ever open.
		int maxCode = Math.Pow( 10, combo.GetLockDigits() ) - 1;

		if ( code < 0 || code > maxCode )
			return;

		bool wasLocked = combo.IsLocked();

		combo.m_CombinationLocked = code;
		combo.SetCombination( code );

#ifndef DAYZ_1_30
		if ( wasLocked )
			combo.ShuffleLock(); //! protected as of 1.30, dials just won't reshuffle visually there
#endif

		combo.CheckLockedStateServer();
		combo.Synchronize();
	}

	//! The locked combination is not among the lock's net-synced variables -
	//! only the dialled one is - so the client cannot read it and has to ask.
	private void Exec_GetCode( Object target, PlayerIdentity ident )
	{
		CombinationLock combo = GetCombinationLock( target );

		if ( !combo )
			return;

		if ( !ident )
		{
			OnObjectActionResult( JMESPObjectAction.GetCode, combo.m_CombinationLocked );
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( JMESPObjectAction.GetCode );
		rpc.Write( combo.m_CombinationLocked );
		rpc.Send( NULL, JMESPModuleRPC.ObjectActionResult, true, ident );
	}

	private void RPC_ObjectAction( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		int action;
		if ( !ctx.Read( action ) )
			return;

		int ivalue;
		if ( !ctx.Read( ivalue ) )
			return;

		float fvalue;
		if ( !ctx.Read( fvalue ) )
			return;

		string permission = PermissionForAction( action );

		if ( permission == "" )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( permission, senderRPC, instance ) )
			return;

		Exec_ObjectAction( action, ivalue, fvalue, obj, senderRPC, instance );
	}

	private void RPC_ObjectActionResult( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		int action;
		if ( !ctx.Read( action ) )
			return;

		int ivalue;
		if ( !ctx.Read( ivalue ) )
			return;

		OnObjectActionResult( action, ivalue );
	}

	private void OnObjectActionResult( int action, int ivalue )
	{
		if ( action != JMESPObjectAction.GetCode )
			return;

		g_Game.CopyToClipboard( ivalue.ToString() );

		COTCreateLocalAdminNotification( new StringLocaliser( "STR_COT_ESP_MODULE_NOTIFY_CODE_COPIED", ivalue.ToString() ) );
	}

	void DeleteObject( int networkLow, int networkHigh )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( networkLow );
		rpc.Write( networkHigh );
		rpc.Send( NULL, JMESPModuleRPC.DeleteObject, true, NULL );

		m_RemoveDeleted = true;
	}

	void DeleteObject( Object target )
	{
		Exec_DeleteObject( target, NULL );

		m_RemoveDeleted = true;
	}

	private void Exec_DeleteObject( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		string obtype = Object.GetDebugName( target );

		vector transform[4];
		target.GetTransform( transform );

		//! Captured before the delete - the entry reads the target's own
		//! state to know what to bring back.
		JMActionHistory.Push( new JMDeleteHistoryEntry( target ) );

		g_Game.ObjectDelete( target );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + obtype + " position=" + transform[3].ToString() + " action=delete" );
		SendWebhookColored( "Delete", instance, "Deleted " + obtype + " at " + transform[3].ToString(), JMConstants.WEBHOOK_COLOR_DANGER );
	}

	private void RPC_DeleteObject( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow;
		int netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );

		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Delete", senderRPC, instance ) )
			return;

		Exec_DeleteObject( obj, senderRPC, instance );
	}

	void BaseBuilding_Build( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Build( target, part, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( part );
			rpc.Send( NULL, JMESPModuleRPC.BaseBuilding_Build, true, NULL );
		}
	}

	private void Exec_BaseBuilding_Build( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		bool requireMaterials = true;
		if ( !IsMissionOffline() )
			requireMaterials = !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Build.MaterialsNotRequired", ident, instance );
		
		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

		target.GetConstruction().COT_BuildRequiredParts( part_name, player, requireMaterials );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=built part=" + part_name + " required_materials=" + requireMaterials );
		SendWebhookColored( "BB_Build", instance, "Built the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_ESP );
	}

	private void RPC_BaseBuilding_Build( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Build", senderRPC, instance ) )
			return;

		BaseBuildingBase bb;
		if ( Class.CastTo( bb, obj ) )
			Exec_BaseBuilding_Build( bb, part_name, senderRPC, instance );
	}

	void BaseBuilding_Dismantle( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Dismantle( target, part, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( part );
			rpc.Send( NULL, JMESPModuleRPC.BaseBuilding_Dismantle, true, NULL );
		}
	}

	private void Exec_BaseBuilding_Dismantle( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

		target.GetConstruction().COT_DismantleRequiredParts( part_name, player );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=dismantle part=" + part_name  );
		SendWebhookColored( "BB_Dismantle", instance, "Dismantled the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_WARNING );
	}

	private void RPC_BaseBuilding_Dismantle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Dismantle", senderRPC, instance ) )
			return;

		BaseBuildingBase bb;
		if ( Class.CastTo( bb, obj ) )
			Exec_BaseBuilding_Dismantle( bb, part_name, senderRPC, instance );
	}

	void BaseBuilding_Repair( BaseBuildingBase target, string part )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_Repair( target, part, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( part );
			rpc.Send( NULL, JMESPModuleRPC.BaseBuilding_Repair, true, NULL );
		}
	}

	private void Exec_BaseBuilding_Repair( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		target.GetConstruction().COT_RepairPart( part_name );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=repair part=" + part_name  );
		SendWebhookColored( "BB_Repair", instance, "Repaired the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")", JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void RPC_BaseBuilding_Repair( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.Repair", senderRPC, instance ) )
			return;

		BaseBuildingBase bb;
		if ( Class.CastTo( bb, obj ) )
			Exec_BaseBuilding_Repair( bb, part_name, senderRPC, instance );
	}

		//! Damage or restore ONE part of a base building object.
	//!
	//! Separate from SetHealth, which only ever addresses an object's global
	//! health: a wall's parts are damage ZONES on one entity, so setting the
	//! whole thing's health would flatten every part at once and there would be
	//! no way to ruin a single gate.
	//!
	//! `health01` is a fraction of that zone's own maximum, because the maximum
	//! differs per part and per material tier - a number of hit points would
	//! mean something different on every row of the menu.
	void BaseBuilding_SetPartHealth( BaseBuildingBase target, string part, float health01 )
	{
		if ( IsMissionOffline() )
		{
			Exec_BaseBuilding_SetPartHealth( target, part, health01, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Write( part );
			rpc.Write( health01 );
			rpc.Send( NULL, JMESPModuleRPC.BaseBuilding_SetPartHealth, true, NULL );
		}
	}

	private void Exec_BaseBuilding_SetPartHealth( BaseBuildingBase target, string part_name, float health01, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		//! The part is named by its CONSTRUCTION part name; the health lives on
		//! the damage zone that part maps to, and only the damage system knows
		//! which that is.
		string damageZone;

		if ( !DamageSystem.GetDamageZoneFromComponentName( target, part_name, damageZone ) )
			return;

		float maxHealth = target.GetMaxHealth( damageZone, "Health" );

		if ( maxHealth <= 0 )
			return;

		target.SetHealth( damageZone, "Health", Math.Clamp( health01, 0, 1 ) * maxHealth );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=parthealth part=" + part_name + " value=" + health01 );
		SendWebhookColored( "BB_Repair", instance, "Set the part \"" + part_name + "\" of \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") to " + Math.Round( health01 * 100 ) + "% health", JMConstants.WEBHOOK_COLOR_WARNING );
	}

	private void RPC_BaseBuilding_SetPartHealth( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		string part_name;
		if ( !ctx.Read( part_name ) )
			return;

		float health01;
		if ( !ctx.Read( health01 ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.BaseBuilding.SetHealth", senderRPC, instance ) )
			return;

		BaseBuildingBase bb;
		if ( Class.CastTo( bb, obj ) )
			Exec_BaseBuilding_SetPartHealth( bb, part_name, health01, senderRPC, instance );
	}

	void Vehicle_Unstuck( Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_Vehicle_Unstuck( target, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Send( NULL, JMESPModuleRPC.Vehicle_Unstuck, true, NULL );
		}
	}

	private void Exec_Vehicle_Unstuck( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		Transport transport;
		if ( Class.CastTo( transport, target ) )
		{
			CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition(transport, transport.GetPosition());
		}

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=Unstuck " );
		SendWebhookColored( "Vehicle_Unstuck", instance, "Unstuck " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_INFO );
	}

	private void RPC_Vehicle_Unstuck( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Car.Unstuck", senderRPC, instance ) )
			return;

		Exec_Vehicle_Unstuck( obj, senderRPC, instance );
	}

	void Vehicle_Refuel( Object target)
	{
		if ( IsMissionOffline() )
		{
			Exec_Vehicle_Refuel( target, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Send( NULL, JMESPModuleRPC.Vehicle_Refuel, true, NULL );
		}
	}

	private void Exec_Vehicle_Refuel( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		CommunityOnlineToolsBase.Refuel(target);

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=refuel" );
		SendWebhookColored( "Vehicle_Refuel", instance, "Refuelled " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void RPC_Vehicle_Refuel( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Car.Refuel", senderRPC, instance ) )
			return;

		Exec_Vehicle_Refuel( obj, senderRPC, instance );
	}

	void Heal( Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_Heal( target, NULL );
		} else
		{
			//! Never target a ScriptRPC at a world object - see SetHealth() above.
			int netLow, netHigh;
			target.GetNetworkID( netLow, netHigh );

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( netLow );
			rpc.Write( netHigh );
			rpc.Send( NULL, JMESPModuleRPC.Heal, true, NULL );
		}
	}

	private void Exec_Heal( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		bool allowDamage = target.GetAllowDamage();

		if (!allowDamage)
			target.SetAllowDamage(true);

		//! Captured before the heal - the entry reads the target's own
		//! current health (and, for a player, energy/water) to know what to
		//! restore.
		EntityAI healEntity;
		if ( Class.CastTo( healEntity, target ) )
			JMActionHistory.Push( new JMHealHistoryEntry( healEntity ) );

		CommunityOnlineToolsBase.HealEntityRecursive(target);

		PlayerBase player;
		if (Class.CastTo(player, target))
		{
			if ( player.GetBleedingManagerServer() )
				player.GetBleedingManagerServer().RemoveAllSources();

			player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
			player.COTRemoveAllDiseases();

			player.GetStatEnergy().Set( player.GetStatEnergy().GetMax() );
			player.GetStatWater().Set( player.GetStatWater().GetMax() );
		}

		if (!allowDamage)
			target.SetAllowDamage(false);

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=heal" );
		SendWebhookColored( "Heal", instance, "Healed " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition(), JMConstants.WEBHOOK_COLOR_SUCCESS );
	}

	private void RPC_Heal( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow, netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		if ( !senderRPC )
			return;

		Object obj = g_Game.GetObjectByNetworkId( netLow, netHigh );
		if ( !obj )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Heal", senderRPC, instance ) )
			return;

		Exec_Heal( obj, senderRPC, instance );
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
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( name );
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMESPModuleRPC.MakeItemSet, true, NULL );
	}

	private void RPC_MakeItemSet( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Items.CreateSet", senderRPC, instance ) )
			return;
		
		string name;
		if ( !ctx.Read( name ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;

		Exec_MakeItemSet( name, objects, instance );
	}

	private void Exec_MakeItemSet( string name, set< Object > objects, JMPlayerInstance instance )
	{
	#ifdef DZ_Expansion_Core
		// Use Expansion loadout format
		array< ref ExpansionPrefab > loadouts = new array< ref ExpansionPrefab >;

		foreach ( Object obj: objects )
		{
			EntityAI entity;
			if ( Class.CastTo( entity, obj ) )
			{
				ExpansionPrefab expPrefab = JMCompensationHelper.CreateExpansionLoadout( entity );
				if ( expPrefab )
					loadouts.Insert( expPrefab );
			}
		}

		if ( loadouts.Count() > 0 )
		{
			string loadoutJSON;
			string errorMsg;

			foreach ( ExpansionPrefab prefabItem: loadouts )
			{
				string singleJSON;
				if ( JsonFileLoader<ExpansionPrefab>.MakeData( prefabItem, singleJSON, errorMsg ) )
				{
					if ( loadoutJSON )
						loadoutJSON += "\n\n";
					loadoutJSON += singleJSON;
				}
			}

			if ( !errorMsg )
			{
				if ( !FileExist( JMConstants.DIR_LOADOUTS ) )
					MakeDirectory( JMConstants.DIR_LOADOUTS );

				string filepath = JMConstants.DIR_LOADOUTS + name + ".json";
				FileHandle file = OpenFile( filepath, FileMode.WRITE );
				if ( file )
				{
					FPrintln( file, loadoutJSON );
					CloseFile( file );

					GetCommunityOnlineToolsBase().Log( instance, "Created Expansion loadout set '" + name + "'" );
					SendWebhookColored( "Create", instance, "Created Expansion loadout set '" + name + "'", JMConstants.WEBHOOK_COLOR_INFO );

					if ( m_LoadoutModule )
						m_LoadoutModule.Load();
				}
			}
		}
	#else
		// Fallback to legacy format
		if ( !m_LoadoutModule )
			Class.CastTo( m_LoadoutModule, GetModuleManager().GetModule( JMLoadoutModule ) );

		if ( m_LoadoutModule )
			m_LoadoutModule.Exec_CreateLoadoutSet( name, objects, instance );
	#endif
	}

	void DuplicateSelected()
	{
		ScriptRPC rpc = new ScriptRPC();
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMESPModuleRPC.DuplicateAll, true, NULL );
	}

	private void RPC_DuplicateAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.DuplicateAll", senderRPC, instance ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;
		
		Exec_DuplicateAll( objects, instance );
	}

	private void Exec_DuplicateAll( set< Object > objects, JMPlayerInstance instance )
	{
	}

	void DeleteSelected()
	{
		if (!g_Game.IsMultiplayer())
		{
			auto objects = new set<Object>;
			auto selectedObjs = JM_GetSelected().GetObjects();
			foreach (auto selectedObj: selectedObjs) objects.Insert(selectedObj.obj);
			Exec_DeleteAll(objects, GetPermissionsManager().GetClientPlayer());
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			JM_GetSelected().SerializeObjects( rpc );
			rpc.Send( NULL, JMESPModuleRPC.DeleteAll, true, NULL );
		}

		JMScriptInvokers.ON_DELETE_ALL.Invoke();

		JM_GetSelected().ClearObjects();

		m_RemoveDeleted = true;
	}

	private void RPC_DeleteAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.DeleteAll", senderRPC, instance ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;

		if ( GetPermissionsManager().HasPermission( "Loadouts.Backup", senderRPC, instance ) )
		{
			if (!m_LoadoutModule)
				Class.CastTo(m_LoadoutModule, GetModuleManager().GetModule(JMLoadoutModule));
			
			m_LoadoutModule.Exec_CreateDeletionBackup(objects, instance);
		}
		
		Exec_DeleteAll( objects, instance );
	}

	private void Exec_DeleteAll( set< Object > objects, JMPlayerInstance instance )
	{
		int removed = 0;
		int count = objects.Count();
		
		int i = objects.Count();
		while ( i > 0 )
		{
			Object obj = objects[i  - 1];
			objects.Remove( i - 1 );

			if ( obj != NULL )
			{
				vector transform[4];
				obj.GetTransform( transform );
				string obtype = Object.GetDebugName( obj );

				GetCommunityOnlineToolsBase().Log( instance, "ESP index=" + ( count - i ) + " target=" + obtype + " position=" + transform[3].ToString() + " action=delete" );

				g_Game.ObjectDelete( obj );
				removed++;
			}

			i = objects.Count();
		}

		if ( removed > 0 )
		{
			GetCommunityOnlineToolsBase().Log( instance, "ESP action=delete_all count=" + removed + " attempted=" + count );
			SendWebhookColored( "DeleteAll", instance, "Performed a delete on " + removed + " objects.", JMConstants.WEBHOOK_COLOR_CRITICAL );
		}
	}

	void MoveToCursor( vector cursor )
	{
		if (!g_Game.IsMultiplayer())
		{
			auto objects = new set<Object>;
			auto selectedObjs = JM_GetSelected().GetObjects();
			foreach (auto selectedObj: selectedObjs) objects.Insert(selectedObj.obj);
			Exec_MoveToCursor(cursor, objects, GetPermissionsManager().GetClientPlayer());
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( cursor );
			JM_GetSelected().SerializeObjects( rpc );
			rpc.Send( NULL, JMESPModuleRPC.MoveToCursor, true, NULL );
		}
	}

	private void RPC_MoveToCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !senderRPC )
			return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.MoveToCursor", senderRPC, instance ) )
			return;
		
		vector cursor;
		if ( !ctx.Read( cursor ) )
			return;

		set< Object > objects = new set< Object >;
		if ( !JM_GetSelected().DeserializeObjects( ctx, objects ) )
			return;
		
		Exec_MoveToCursor( cursor, objects, instance );
	}

	private void Exec_MoveToCursor( vector cursor, set< Object > objects, JMPlayerInstance instance )
	{
		int moved = 0;
		int count = objects.Count();
		
		int i = objects.Count();
		while ( i > 0 )
		{
			Object obj = objects[i  - 1];
			objects.Remove( i - 1 );

			if ( obj != NULL )
			{
				vector transform[4];
				obj.GetTransform( transform );
				string obtype = Object.GetDebugName( obj );

				GetCommunityOnlineToolsBase().Log( instance, "ESP index=" + ( count - i ) + " target=" + obtype + " position=" + transform[3].ToString() + " action=MoveToCursor" );

				EntityAI ent = EntityAI.Cast(obj);
				if (ent)
					CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition(ent, cursor);
				else
					obj.SetPosition(cursor);
				moved++;
			}

			i = objects.Count();
		}

		if ( moved > 0 )
		{
			GetCommunityOnlineToolsBase().Log( instance, "ESP action=move_to_cursor count=" + moved + " attempted=" + count );
			SendWebhookColored( "MoveToCursor", instance, "Performed a move to cursor on " + moved + " objects.", JMConstants.WEBHOOK_COLOR_ESP );
		}
	}

	void CopyToClipboardRaw()
	{
		string clipboardOutput= "";
		set< ref JMSelectedObject > JMobjects = JM_GetSelected().GetObjects();
		for(int i=0; i < JMobjects.Count(); i++)
		{
			if (i > 0)
				clipboardOutput = clipboardOutput + "\n";

			clipboardOutput = clipboardOutput + JMobjects[i].obj.GetType() + "\n";
			
			EntityAI ent;
			if (!Class.CastTo(ent, JMobjects[i].obj))
				continue;

			if (ent.IsEmpty())
				continue;
			
			for (int k=0; k < ent.GetInventory().AttachmentCount(); k++)
			{
				clipboardOutput = clipboardOutput + ent.GetInventory().GetAttachmentFromIndex( k ).GetType() + "\n";
			}

			CargoBase cargo = ent.GetInventory().GetCargo();
			if(!cargo)
				continue;

			for(int j=0; j < cargo.GetItemCount(); j++)
			{
				clipboardOutput = clipboardOutput + cargo.GetItem(j).GetType() + "\n";
			}
		}

		g_Game.CopyToClipboard(clipboardOutput);
	}

	void CopyToClipboardMarket()
	{
#ifdef DZ_Expansion_Market
		string categoryJSON;
		set<ref JMSelectedObject> selectedObjects = JM_GetSelected().GetObjects();

		auto category = new ExpansionMarketCategory();
		category.Defaults();
		category.DisplayName = "My Category Name";

		foreach (JMSelectedObject selectedObj: selectedObjects)
		{
			TStringArray atts = {};

			EntityAI ent;
			if (Class.CastTo(ent, selectedObj.obj))
			{
				for (int i = 0; i < ent.GetInventory().AttachmentCount(); ++i)
				{
					EntityAI att = ent.GetInventory().GetAttachmentFromIndex(i);
					atts.Insert(att.GetType());
				}
			}			

			auto item = new ExpansionMarketItem(-1, selectedObj.obj.GetType(), 100, 100, 1, 1, atts);
			category.Items.Insert(item);
		}

		string errorMsg;
		if (JsonFileLoader<ExpansionMarketCategory>.MakeData(category, categoryJSON, errorMsg))
			g_Game.CopyToClipboard(categoryJSON);
		else
			COTCreateLocalAdminNotification(new StringLocaliser(errorMsg));
#endif
	}

	void CopyToClipboardSpawnableTypes()
	{
		string clipboardOutput= "";
		
		clipboardOutput += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n";
		clipboardOutput += "<spawnabletypes>\n";

		set< ref JMSelectedObject > JMobjects = JM_GetSelected().GetObjects();
		for(int i=0; i < JMobjects.Count(); i++)
		{
			clipboardOutput += "	<type name=\""+JMobjects[i].obj.GetType()+"\">\n";
			
			EntityAI ent;
			if (Class.CastTo(ent, JMobjects[i].obj))
			{
				for (int k=0; k < ent.GetInventory().AttachmentCount(); k++)
				{
					clipboardOutput += "		<attachments chance=\"1.00\">\n";
					clipboardOutput += "			<item name=\""+ent.GetInventory().GetAttachmentFromIndex( k ).GetType()+"\" chance=\"1.00\" />\n";
					clipboardOutput += "		</attachments>\n";
				}

				CargoBase cargo = ent.GetInventory().GetCargo();
				if(cargo)
				{
					for(int j=0; j < cargo.GetItemCount(); j++)
					{
						clipboardOutput += "		<cargo chance=\"1.00\">\n";
						clipboardOutput += "			<item name=\""+cargo.GetItem(j).GetType()+"\" />\n";
						clipboardOutput += "		</cargo>\n";
					}
				}
			}

			clipboardOutput += "	</type>\n";
		}

		clipboardOutput += "</spawnabletypes>\n";
		g_Game.CopyToClipboard(clipboardOutput);
	}

#ifdef DZ_Expansion_Core
	bool CopyToClipboardExpLoadout(typename type)
	{
		auto selected = JM_GetSelected();
		set<ref JMSelectedObject> selectedObjs = new set<ref JMSelectedObject>;

		switch (type)
		{
			case JMPlayerInstance:
				selectedObjs = new set<ref JMSelectedObject>();
				auto uids = selected.GetPlayers();
				foreach (string uid: uids)
				{
					auto inst = GetPermissionsManager().GetPlayer(uid);
					selectedObjs.Insert(new JMSelectedObject(inst.PlayerObject));
				}
				break;

			case JMSelectedObject:
			default:
				selectedObjs = selected.GetObjects();
				break;
		}

		string loadoutsJSON;
		string errorMsg;

		foreach (JMSelectedObject selectedObj: selectedObjs)
		{
			ExpansionPrefab expPrefab = new ExpansionPrefab();

			EntityAI entity;
			if (Class.CastTo(entity, selectedObj.obj))
			{
				if (entity.IsMan())
				{
					AddChildrenToExpLoadoutRecursive(expPrefab, entity);
				}
				else
				{
					expPrefab.ClassName = entity.GetType();
					AddToExpLoadoutRecursive(expPrefab, entity);
				}
			}

			string loadoutJSON;
			if (JsonFileLoader<ExpansionPrefab>.MakeData(expPrefab, loadoutJSON, errorMsg))
			{
				if (loadoutsJSON)
					loadoutsJSON += "\n\n";

				loadoutsJSON += loadoutJSON;
			}
			else
			{
				//! Abort
				break;
			}
		}

		if (errorMsg)
		{
			COTCreateLocalAdminNotification(new StringLocaliser(errorMsg));
		}
		else
		{
			g_Game.CopyToClipboard(loadoutsJSON);
		}

		return errorMsg == string.Empty;
	}

	void AddChildrenToExpLoadoutRecursive(ExpansionPrefab prefab, EntityAI entity)
	{
		auto inventory = entity.GetInventory();
		int i;
		EntityAI item;
		auto il = new InventoryLocation();

		for (i = 0; i < inventory.AttachmentCount(); ++i)
		{
			item = inventory.GetAttachmentFromIndex(i);
			item.GetInventory().GetCurrentInventoryLocation(il);
			string slotName = InventorySlots.GetSlotName(il.GetSlot());
			prefab = ExpansionPrefab.Cast(prefab.BeginAttachment(item.GetType(), slotName));
			AddToExpLoadoutRecursive(prefab, item);
			prefab = ExpansionPrefab.Cast(prefab.End());
		}

		auto cargo = inventory.GetCargo();
		if (cargo)
		{
			for (i = 0; i < cargo.GetItemCount(); ++i)
			{
				item = cargo.GetItem(i);
				prefab = ExpansionPrefab.Cast(prefab.BeginCargo(item.GetType()));
				AddToExpLoadoutRecursive(prefab, item);
				prefab = ExpansionPrefab.Cast(prefab.End());
			}
		}
	}

	void AddToExpLoadoutRecursive(ExpansionPrefab prefab, EntityAI item)
	{
		prefab.Chance = 1.0;

		if (item.HasQuantity())
		{
			float quantity01 = item.GetQuantityNormalized();
			prefab.SetQuantity(quantity01, quantity01);
		}

		AddChildrenToExpLoadoutRecursive(prefab, item);
	}
#endif
}
