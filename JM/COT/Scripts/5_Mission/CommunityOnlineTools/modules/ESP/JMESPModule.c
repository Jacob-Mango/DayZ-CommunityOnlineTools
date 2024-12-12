enum JMESPState
{
	UNKNOWN = 0,
	Update,
	View,
	Remove
};

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
			m_Canvas = CanvasWidget.Cast(GetGame().GetWorkspace().CreateWidgets("JM/COT/GUI/layouts/esp_canvas.layout"));
	}

	bool HasCanvas()
	{
		return m_Canvas != null;
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
		screen_pos = GetGame().GetScreenPosRelative(pWorldPos);
		isInBounds = screen_pos[0] >= 0 && screen_pos[0] <= 1 && screen_pos[1] >= 0 && screen_pos[1] <= 1 && screen_pos[2] >= 0;
		//! get size of parent widget
		m_Canvas.GetScreenSize(parent_width, parent_height);
		
		//! calculate current position from relative pos and parent widget size
		screen_pos[0] = screen_pos[0] * parent_width;
		screen_pos[1] = screen_pos[1] * parent_height;
		
		return screen_pos;
	}
};

class JMESPLimb
{
	string Bone1;
	string Bone2;

	void JMESPLimb(string bone1, string bone2)
	{
		Bone1 = bone1;
		Bone2 = bone2;
	}
};

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
			vector ori = GetGame().GetCurrentCameraDirection().VectorToAngles();
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
};

class JMESPModule: JMRenderableModuleBase
{
	private ref array< Object > m_SelectedObjects;
	
	private ref array< ref JMESPMeta > m_ActiveESPObjects;

	private ref array< ref JMESPMeta > m_ESPToCreate;
	private ref array< JMESPMeta > m_ESPToDestroy;

	private ref map< Object, JMESPMeta > m_MappedESPObjects;

	private ref array< ref JMESPViewType > m_ViewTypes;
	private ref map<typename, JMESPViewType> m_ViewTypesByType;

	private bool m_IsCreatingWidgets;
	private bool m_IsDestroyingWidgets;
	private bool m_IknowWhatIamDoing;

	string Filter;

	float ESPRadius;
	int ESPUpdateTime;
	private bool DrawPlayerSkeletonsEnabled;
	bool DrawPlayerSkeletonsIncludingMyself;
	float SkeletonLineThickness = 1;

	private JMESPState m_CurrentState = JMESPState.Remove;
	private bool m_StateChanged = false;

	ref JMESPCanvas m_ESPCanvas;

	private JMLoadoutModule m_LoadoutModule;

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

		GetPermissionsManager().RegisterPermission( "ESP.Object.Car.Unstuck" );
		GetPermissionsManager().RegisterPermission( "ESP.Object.Car.Refuel" );

		GetPermissionsManager().RegisterPermission( "ESP.Object.Heal" );
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

	override string GetTitle()
	{
		return "#STR_COT_ESP_MODULE_NAME";
	}
	
	override string GetIconName()
	{
		return "JM\\COT\\GUI\\textures\\modules\\ESP.paa";
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

	JMESPViewType GetViewType(typename type)
	{
		return m_ViewTypesByType[type];
	}

	bool IncludeImmovable()
	{
		if (m_ViewTypesByType[JMESPViewTypePlainObject].View)
			return true;

		if (m_ViewTypesByType[JMESPViewTypeRock].View)
			return true;

		if (m_ViewTypesByType[JMESPViewTypeBush].View)
			return true;

		if (m_ViewTypesByType[JMESPViewTypeTree].View)
			return true;

		if (m_ViewTypesByType[JMESPViewTypeImmovable].View)
			return true;

		return false;
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

	override void OnMissionFinish()
	{
		for (int j = 0; j < m_ActiveESPObjects.Count(); j++ )
		{
			m_ActiveESPObjects[j].Destroy();
		}

		m_ActiveESPObjects.Clear();
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

	override void OnUpdate(float timeslice)
	{
		if (!DrawPlayerSkeletonsEnabled || !m_ESPCanvas || !m_ESPCanvas.HasCanvas())
			return;

		auto spectatorCamera = JMSpectatorCamera.Cast(CurrentActiveCamera);
		Man gamePlayer = GetGame().GetPlayer();

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

			vector btm = GetGame().GetScreenPosRelative(human.GetPosition());
			if (btm[2] < 0 || btm[2] > ESPRadius)
				continue;

			vector headPos = human.GetBonePositionWS(human.GetBoneIndexByName("head"));
			vector top = GetGame().GetScreenPosRelative(headPos);
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
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CreateNewWidgets, 10, false);
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
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DestroyOldWidgets, 10, false);
		else
			m_IsDestroyingWidgets = false;

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
					GetGame().IsBoxCollidingGeometry(center, vector.Zero, extents, ObjIntersectView, ObjIntersectFire, excluded, collided);

					foreach (auto obj : collided)
					{
						objects.Insert(obj);
					}

					Sleep(100);
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

				set<Object> objects = new set<Object>;
				set<Object> addedObjects = new set<Object>;

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

						string type = JMESPMeta.GetObjectType(obj);
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

						if ( isUsingFilter && !type.Contains( filter ) )
							continue;

						JMESPMeta meta = m_MappedESPObjects.Get( obj );
						if ( meta != NULL )
						{
							#ifdef JM_COT_ESP_DEBUG
							bool metaIsValid = meta.IsValid();
							#ifdef COT_DEBUGLOGS
							Print( "-" + meta.ClassName() + "::IsValid() = " + metaIsValid );
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
								Print( "-" + validViewTypes[j].ClassName() + "::IsValid( obj = " + Object.GetDebugName( obj ) + ", out = " + meta + " ) = " + viewTypeIsValid );
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
			rpc.Send( NULL, JMESPModuleRPC.Log, true, NULL );
		}
	}

	private void Exec_Log( string log, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		GetCommunityOnlineToolsBase().Log( ident, "ESP: " + log );
		SendWebhook( "Log", instance, "Logging ESP action: " + log );
	}

	private void RPC_Log( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			rpc.Send( target, JMESPModuleRPC.SetPosition, true, NULL );
		}
	}

	private void Exec_SetPosition( vector position, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		Transport transport;
		if ( Class.CastTo( transport, target ) )
		{
			CommunityOnlineToolsBase.ForceTransportPositionAndOrientation(transport, position, transport.GetOrientation());
		}
		else
		{
			target.SetPosition( position );
		}
		
		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=position value=" + position );
		SendWebhook( "Position", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") position to " + position.ToString() );
	}

	private void RPC_SetPosition( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			rpc.Send( target, JMESPModuleRPC.SetOrientation, true, NULL );
		}
	}

	private void Exec_SetOrientation( vector orientation, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		Transport transport;
		if ( Class.CastTo( transport, target ) )
		{
			CommunityOnlineToolsBase.ForceTransportPositionAndOrientation(transport, transport.GetPosition(), orientation);
		}
		else
		{
			target.SetOrientation( orientation );
		}

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=orientation value=" + orientation );
		SendWebhook( "Orientation", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") orientation to " + orientation.ToString() );
	}

	private void RPC_SetOrientation( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			rpc.Send( target, JMESPModuleRPC.SetHealth, true, NULL );
		}
	}

	private void Exec_SetHealth( float health, string zone, Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( (target.IsInherited(Man) || target.IsInherited(DayZCreature)) && !target.IsAlive() )
			return;

		target.SetHealth( health );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=health value=" + health );
		SendWebhook( "Health", instance, "Set \"" + target.GetDisplayName() + "\" (" + target.GetType() + ") health to " + health );
	}

	private void RPC_SetHealth( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		float health;
		if ( !ctx.Read( health ) )
			return;

		string zone;
		//! TODO: Setting zone is not implemented in GUI, always sets global health
		//if ( !ctx.Read( zone ) )
			//return;

		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.SetHealth", senderRPC, instance ) )
			return;

		Exec_SetHealth( health, zone, target, senderRPC, instance );
	}

	void DeleteObject( int networkLow, int networkHigh )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( networkLow );
		rpc.Write( networkHigh );
		rpc.Send( NULL, JMESPModuleRPC.DeleteObject, true, NULL );
	}

	void DeleteObject( Object target )
	{
		Exec_DeleteObject( target, NULL );
	}

	private void Exec_DeleteObject( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		if ( !target )
			return;

		string obtype = Object.GetDebugName( target );

		vector transform[4];
		target.GetTransform( transform );

		GetGame().ObjectDelete( target );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + obtype + " position=" + transform[3].ToString() + " action=delete" );
		SendWebhook( "Delete", instance, "Deleted " + obtype + " at " + transform[3].ToString() );
	}

	private void RPC_DeleteObject( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		int netLow;
		int netHigh;
		if ( !ctx.Read( netLow ) || !ctx.Read( netHigh ) )
			return;

		Object obj = GetGame().GetObjectByNetworkId( netLow, netHigh );

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
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( part );
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Build, true, NULL );
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
		SendWebhook( "BB_Build", instance, "Built the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Build( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Dismantle, true, NULL );
		}
	}

	private void Exec_BaseBuilding_Dismantle( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		PlayerBase player;
		Class.CastTo( player, GetPlayerObjectByIdentity( ident ) );

		target.GetConstruction().COT_DismantleRequiredParts( part_name, player );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=dismantle part=" + part_name  );
		SendWebhook( "BB_Dismantle", instance, "Dismantled the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Dismantle( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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
			rpc.Send( target, JMESPModuleRPC.BaseBuilding_Repair, true, NULL );
		}
	}

	private void Exec_BaseBuilding_Repair( BaseBuildingBase target, string part_name, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		target.GetConstruction().COT_RepairPart( part_name );

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=repair part=" + part_name  );
		SendWebhook( "BB_Repair", instance, "Repaired the part \"" + part_name + "\" for \"" + target.GetDisplayName() + "\" (" + target.GetType() + ")" );
	}

	private void RPC_BaseBuilding_Repair( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
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

	
	void Vehicle_Unstuck( Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_Vehicle_Unstuck( target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( target, JMESPModuleRPC.Vehicle_Unstuck, true, NULL );
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
		SendWebhook( "Vehicle_Unstuck", instance, "Unstuck " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition() );
	}

	private void RPC_Vehicle_Unstuck( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Car.Unstuck", senderRPC, instance ) )
			return;

		Exec_Vehicle_Unstuck( target, senderRPC, instance );
	}

	void Vehicle_Refuel( Object target)
	{
		if ( IsMissionOffline() )
		{
			Exec_Vehicle_Refuel( target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( target, JMESPModuleRPC.Vehicle_Refuel, true, NULL );
		}
	}

	private void Exec_Vehicle_Refuel( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		CommunityOnlineToolsBase.Refuel(target);

		GetCommunityOnlineToolsBase().Log( ident, "ESP target=" + target + " action=refuel" );
		SendWebhook( "Vehicle_Refuel", instance, "Refuelled " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition() );
	}

	private void RPC_Vehicle_Refuel( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Car.Refuel", senderRPC, instance ) )
			return;

		Exec_Vehicle_Refuel( target, senderRPC, instance );
	}

	void Heal( Object target )
	{
		if ( IsMissionOffline() )
		{
			Exec_Heal( target, NULL );
		} else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( target, JMESPModuleRPC.Heal, true, NULL );
		}
	}

	private void Exec_Heal( Object target, PlayerIdentity ident, JMPlayerInstance instance = NULL )
	{
		bool allowDamage = target.GetAllowDamage();

		if (!allowDamage)
			target.SetAllowDamage(true);

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
		SendWebhook( "Heal", instance, "Healed " + target.GetDisplayName() + " (" + target.GetType() + ") at " + target.GetPosition() );
	}

	private void RPC_Heal( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "ESP.Object.Heal", senderRPC, instance ) )
			return;

		Exec_Heal( target, senderRPC, instance );
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
	}

	void DuplicateSelected()
	{
		ScriptRPC rpc = new ScriptRPC();
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMESPModuleRPC.DuplicateAll, true, NULL );
	}

	private void RPC_DuplicateAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
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
		ScriptRPC rpc = new ScriptRPC();
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMESPModuleRPC.DeleteAll, true, NULL );

		JMScriptInvokers.ON_DELETE_ALL.Invoke();

		JM_GetSelected().ClearObjects();
	}

	private void RPC_DeleteAll( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
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

				GetGame().ObjectDelete( obj );
				removed++;
			}

			i = objects.Count();
		}

		if ( removed > 0 )
		{
			GetCommunityOnlineToolsBase().Log( instance, "ESP action=delete_all count=" + removed + " attempted=" + count );
			SendWebhook( "DeleteAll", instance, "Performed a delete on " + removed + " objects." );
		}
	}

	void MoveToCursor( vector cursor )
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write( cursor );
		JM_GetSelected().SerializeObjects( rpc );
		rpc.Send( NULL, JMESPModuleRPC.MoveToCursor, true, NULL );
	}

	private void RPC_MoveToCursor( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
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
			SendWebhook( "MoveToCursor", instance, "Performed a move to cursor on " + moved + " objects." );
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

		GetGame().CopyToClipboard(clipboardOutput);
	}

	void CopyToClipboardMarket()
	{
		string clipboardOutput= "";
		set< ref JMSelectedObject > JMobjects = JM_GetSelected().GetObjects();
		
		clipboardOutput = "{\n";
		clipboardOutput += "    \"m_Version\": 12,\n";
		clipboardOutput += "    \"DisplayName\": \"My Category Name\",\n";
		clipboardOutput += "    \"Icon\": \"Deliver\",\n";
		clipboardOutput += "    \"Color\": \"FBFCFEFF\",\n";
		clipboardOutput += "    \"IsExchange\": 0,\n";
		clipboardOutput += "    \"InitStockPercent\": 75.0,\n";
		clipboardOutput += "    \"Items\": [\n";
		for(int i=0; i < JMobjects.Count(); i++)
		{
			clipboardOutput += "        {\n";
			clipboardOutput += "            \"ClassName\": \"" + JMobjects[i].obj.GetType() + "\",\n";
			clipboardOutput += "            \"MaxPriceThreshold\": 100,\n";
			clipboardOutput += "            \"MinPriceThreshold\": 100,\n";
			clipboardOutput += "            \"SellPricePercent\": -1.0,\n";
			clipboardOutput += "            \"MaxStockThreshold\": 1,\n";
			clipboardOutput += "            \"MinStockThreshold\": 1,\n";
			clipboardOutput += "            \"QuantityPercent\": -1,\n";

			EntityAI ent;
			if (Class.CastTo(ent, JMobjects[i].obj))
			{
				if (ent.IsEmpty())
				{
					clipboardOutput += "            \"SpawnAttachments\": [],\n";
				}
				else
				{
					clipboardOutput += "            \"SpawnAttachments\": [\n";
					for (int k=0; k < ent.GetInventory().AttachmentCount(); k++)
					{
						clipboardOutput += "                \""+ent.GetInventory().GetAttachmentFromIndex( k ).GetType() + "\"";
						if ( k+1 < ent.GetInventory().AttachmentCount() )
							clipboardOutput += ",";
						
						clipboardOutput += "\n";
					}
					clipboardOutput += "            ],\n";
				}
			}			

			clipboardOutput += "            \"Variants\": []\n";

			clipboardOutput += "        }";
			if ( i + 1 < JMobjects.Count() )
				clipboardOutput += ",";

			clipboardOutput += "\n";
		}
		clipboardOutput += "    ]\n";
		clipboardOutput += "}";
		GetGame().CopyToClipboard(clipboardOutput);
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
		GetGame().CopyToClipboard(clipboardOutput);
	}
};
