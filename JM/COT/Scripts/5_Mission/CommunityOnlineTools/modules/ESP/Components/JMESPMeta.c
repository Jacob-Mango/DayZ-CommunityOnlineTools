//! One tracked object: what the ESP knows about it and how to reach it.
//!
//! This used to own a panel of controls that unfolded under the tag. It does
//! not any more - every action lives in JMESPActionMenu, reachable by
//! right-clicking either the world tag or the object's row in the ESP form.
//! One menu means one place to add an action and one set of permission checks,
//! instead of a widget tree per visible object that had to be built, refreshed
//! and torn down whether anyone ever opened it or not.
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
	bool m_TargetDeleted;

	//! Which Building door the right-click landed on, from the raycast
	//! component the menu was opened with. -1 when the target isn't a
	//! Building or the click wasn't on any of its doors.
	int m_DoorIndex = -1;

	void JMESPMeta()
	{
		s_JM_Node = s_JM_All.Add(this);

	#ifdef DIAG
		s_JMESPMetaCount++;
	#endif
	}

	void ~JMESPMeta()
	{
		if (!g_Game)
			return;

		#ifdef JM_COT_ESP_DEBUG
		auto trace = CF_Trace_0(this);
		Print( "  widgetHandler = " + widgetHandler );
		#endif

	#ifdef DAYZ_1_28
		DestroyWidget(widgetRoot);
	#endif

		if (s_JM_All)
			s_JM_All.Remove(s_JM_Node);

	#ifdef DIAG
		s_JMESPMetaCount--;
		if (s_JMESPMetaCount <= 0)
			CF_Log.Info("JMESPMeta count: " + s_JMESPMetaCount);
	#endif
	}

	//! Construction parts, for the objects that have any. NULL everywhere else,
	//! which is what keeps the base building page out of a rock's menu.
	map< string, ref JMConstructionPartData > GetConstructionParts()
	{
		return NULL;
	}

	string GetName()
	{
		string displayName = target.GetDisplayName();

		if (displayName == "")
			displayName = GetType();

		return displayName;
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

	string GetType()
	{
		return GetObjectType(target);
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

	void Create( JMESPModule mod )
	{
		module = mod;

		if ( widgetRoot )
			return;

		//! A pooled handler (JMESPWidgetHandler.s_Pool) already has its widget
		//! tree built - reuse it and skip CreateWidgets() entirely. Falls back
		//! to building a fresh one only when the pool has nothing free.
		JMESPWidgetHandler pooled = JMESPWidgetHandler.TakeFromPool();

		if ( pooled )
		{
			widgetHandler = pooled;
			widgetRoot = pooled.GetLayoutRoot();
		}
		else
		{
			if ( !Class.CastTo( widgetRoot, g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/esp_widget.layout", JMStatics.ESP_CONTAINER ) ) )
				return;

			widgetRoot.GetScript( widgetHandler );
			if ( !widgetHandler )
				return;
		}

		widgetHandler.SetInfo( this );
	}

	//! Returns this meta's widget to JMESPWidgetHandler's pool instead of
	//! leaving it orphaned - see the pool's own comment for why that matters
	//! on this engine version specifically. widgetRoot is cleared too: on
	//! DAYZ_1_28 the destructor below still unlinks it directly, which would
	//! otherwise destroy the very widget just handed to the pool for reuse.
	override void Destroy()
	{
		if ( widgetHandler )
		{
			widgetHandler.SetInfo( NULL );
			widgetHandler.ReturnToPool();

			widgetHandler = NULL;
			widgetRoot = NULL;
		}

		delete this;
	}

	bool CanDelete()
	{
		return true;
	}

	//! DEPRECATED. Kept only so third party mods that were written against the
	//! old unfolding action panel still compile - DayZ Expansion overrides this
	//! on JMESPMeta, JMESPMetaBaseBuilding and two meta types of its own.
	//!
	//! Nothing calls it any more: actions live in JMESPActionMenu, which builds
	//! menu rows rather than widgets, so there is no `parent` to hand out. The
	//! overrides are inert, not broken - the actions they used to add are all
	//! reachable from the context menu instead.
	void CreateActions( Widget parent )
	{
	}
}


class JMESPMetaPlayer : JMESPMeta
{
	override bool CanDelete()
	{
		return !target.IsAlive();
	}
}


class JMESPMetaBaseBuilding : JMESPMeta
{
	ref map< string, ref JMConstructionPartData > m_Parts;

	//! DEPRECATED, see JMESPMeta::CreateActions. These stay allocated and empty
	//! so Expansion's UpdateButtonStates override - which indexes all four in
	//! lockstep off m_StateHeaders.Count() - iterates zero times instead of
	//! dereferencing NULL.
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

	//! DEPRECATED, see JMESPMeta::CreateActions. Expansion overrides this to
	//! hide the rows for parts it does not want offered; the menu decides that
	//! for itself now in JMESPActionMenu::BuildConstruction.
	protected void UpdateButtonStates()
	{
	}

	//! Re-read whenever the menu asks, rather than cached off a construction
	//! callback: the menu is the only reader now, and it only exists for as
	//! long as somebody is looking at it.
	override map< string, ref JMConstructionPartData > GetConstructionParts()
	{
		if ( !Class.CastTo( m_BaseBuilding, target ) )
			return NULL;

		m_BaseBuilding.GetConstruction().COT_GetParts( m_Parts, false );

		return m_Parts;
	}
}

//! Kept as the meta types the view types name, even though the actions they
//! used to carry now come from the menu's own type tests.
class JMESPMetaCar : JMESPMeta
{
}

class JMESPMetaBoat : JMESPMeta
{
}

class JMESPMetaTrain : JMESPMeta
{
}
