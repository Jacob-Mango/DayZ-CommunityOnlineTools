
class ObjectEditor extends Module
{
	protected bool m_ObjectEditorActive = false;
	protected bool m_IsDragging;
	Object m_SelectedObject;

	protected ref ObjectMenu m_ObjectMenu;

	string BASE_COM_DIR = "$saves:CommunityOfflineMode";
	string BASE_SCENE_DIR = BASE_COM_DIR + "\\Scenes";

	ref array< ref Object> m_Objects = new array< ref Object>;

	void ObjectEditor()
	{
		MakeDirectory(BASE_COM_DIR);
		MakeDirectory(BASE_SCENE_DIR);
	}

	void ~ObjectEditor()
	{

	}
	
	void addObject( Object trackedObject ) 
	{
		m_Objects.Insert( trackedObject );
	}

	override void Init() 
	{
		super.Init();

		LoadScene();
	}

	override void RegisterKeyMouseBindings() 
	{
		if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;
		
		KeyMouseBinding toggleEditor  = new KeyMouseBinding( GetModuleType(), "ToggleEditor" , "[Shift]+[End]" , "Toggle object editor."            );
		KeyMouseBinding objectSelect  = new KeyMouseBinding( GetModuleType(), "ClickObject"  , "(LMB)+(Click)" , "Selects object on cursor.", true  );
		KeyMouseBinding objectDrag    = new KeyMouseBinding( GetModuleType(), "DragObject"   , "(LMB)+(Drag)"  , "Drag objects on cursor.",   true  );
		KeyMouseBinding objectDelete  = new KeyMouseBinding( GetModuleType(), "DeleteObject" , "[Delete]"	   , "Deletes selected object.",  true  );
		KeyMouseBinding objectGround  = new KeyMouseBinding( GetModuleType(), "GroundObject" , "(Middle Mouse)", "Snaps objects to ground.",  true  );
		KeyMouseBinding sceneSave     = new KeyMouseBinding( GetModuleType(), "ExportScene"  , "CTRL+S"	       , "Saves current scene of objects", true);

		toggleEditor.AddKeyBind( KeyCode.KC_LSHIFT, KeyMouseBinding.KB_EVENT_HOLD    ); 
		toggleEditor.AddKeyBind( KeyCode.KC_END   , KeyMouseBinding.KB_EVENT_RELEASE ); // Press END. Using Release prevents key HOLD spam from onKeyPress (could use ClearKey in onKeyPress however)
		objectDelete.AddKeyBind( KeyCode.KC_DELETE, KeyMouseBinding.KB_EVENT_RELEASE ); // Pretty much making KB_EVENT_PRESS useless since you can just use KB_EVENT_HOLD instead.
		sceneSave.AddKeyBind( KeyCode.KC_LCONTROL,  KeyMouseBinding.KB_EVENT_HOLD );
		sceneSave.AddKeyBind( KeyCode.KC_S, 	 KeyMouseBinding.KB_EVENT_RELEASE );


		objectSelect.AddMouseBind( MouseState.LEFT		, KeyMouseBinding.MB_EVENT_CLICK ); // Left Click
		objectDrag.  AddMouseBind( MouseState.LEFT 		, KeyMouseBinding.MB_EVENT_DRAG  );
		objectGround.AddMouseBind( MouseState.MIDDLE	, KeyMouseBinding.MB_EVENT_CLICK );
		
		// RegisterKeyMouseBinding( toggleEditor );
		// RegisterKeyMouseBinding( objectSelect );
		// RegisterKeyMouseBinding( objectDrag   );
		// RegisterKeyMouseBinding( objectDelete );
		// RegisterKeyMouseBinding( objectGround );
		// RegisterKeyMouseBinding( sceneSave );
	}

	void ExportScene() 
	{
		string toCopy = "Object obj; \n";

		foreach( Object m_object : m_Objects ) 
		{
			toCopy = toCopy + "obj = GetGame().CreateObject(\"" + m_object.GetType() + "\", \"" + VectorToString( m_object.GetPosition() ) + "\");\nobj.SetOrientation(\"" + VectorToString( m_object.GetOrientation() ) + "\");\nobj.SetPosition(\"" + VectorToString( m_object.GetPosition() ) + "\");\n";
			//toCopy = toCopy + "GetGame().CreateObject(\"" + m_object.GetType() + "\", \"" + VectorToString( m_object.GetPosition() ) + "\").SetOrientation(\"" + VectorToString( m_object.GetOrientation() ) + "\");\n";
		}

		GetGame().CopyToClipboard( toCopy );

		Message( "Copied to clipboard" );
	}

	void SaveScene() 
	{
		
	}

	void LoadScene() 
	{
		
	}

	void EditorState( bool state ) 
	{
		if ( m_ObjectEditorActive == state ) 
		{
			return;
		}

		m_ObjectEditorActive = state;

		if ( m_ObjectEditorActive ) 
		{	
			GetPlayer().MessageStatus("Object Editor Enabled");
		} 
		else 
		{
			GetPlayer().MessageStatus("Object Editor Disabled");
		}
	}
	
	void ToggleEditor()
	{
		m_ObjectEditorActive = !m_ObjectEditorActive;
		
		if ( m_ObjectEditorActive ) 
		{	
			GetPlayer().MessageStatus("Object Editor Enabled");
		} 
		else 
		{
			GetPlayer().MessageStatus("Object Editor Disabled");
		}
	}

	bool IsEditing() 
	{
		return m_ObjectEditorActive;
	}

	void SelectObject( Object object )
	{
		if ( ( ( m_SelectedObject != NULL ) && ( m_SelectedObject == object ) ) || object.IsInherited( PlayerBase ) )
		{
			return;
		}
		
		m_SelectedObject = object;
	}

	void DeselectObject() 
	{
		m_SelectedObject = NULL;
	}
	
	void DragObject() 
	{
		if ( !m_ObjectEditorActive )
		{
			return;
		}

		if ( m_SelectedObject )
		{
			vector dir = GetGame().GetPointerDirection();

			vector from = GetGame().GetCurrentCameraPosition();

			vector to = from + ( dir * 10000 );

			vector contact_pos;
			vector contact_dir;
			vector bbox;

			int contact_component;

			m_SelectedObject.GetCollisionBox( bbox );

			if ( DayZPhysics.RaycastRV( from, to, contact_pos, contact_dir, contact_component, NULL, m_SelectedObject, GetPlayer(), false, false ) )
			{
				//vector oOrgPos = m_SelectedObject.GetPosition();
				//float fSurfaceHight = GetGame().SurfaceY( oOrgPos [ 0 ], oOrgPos [ 2 ] );

				//float nHightOffsetToGround = oOrgPos [ 1 ] - fSurfaceHight; // todo: store offset of existing objects for later use. for snap to ground perhaps?

				//contact_pos [ 1 ] = contact_pos [ 1 ] + nHightOffsetToGround;

				m_SelectedObject.SetPosition( contact_pos );
				m_SelectedObject.PlaceOnSurface();

				ForceTargetCollisionUpdate( m_SelectedObject );

				ObjectInfoMenu.infoPosX.SetText( m_SelectedObject.GetPosition()[0].ToString() );
				ObjectInfoMenu.infoPosY.SetText( m_SelectedObject.GetPosition()[1].ToString() );
				ObjectInfoMenu.infoPosZ.SetText( m_SelectedObject.GetPosition()[2].ToString() );
			}
		}
	}

	void ClickObject() 
	{
		if ( !m_ObjectEditorActive )
		{
			return;
		}
		Widget widgetCursor = GetGame().GetUIManager().GetWidgetUnderCursor();

		if ( widgetCursor && widgetCursor.GetName() != "EditorMenu" ) 
		{
			return;
		}

		vector dir = GetGame().GetPointerDirection();
		vector from = GetGame().GetCurrentCameraPosition();
		vector to = from + ( dir * 100 );

		set< Object > objects = GetObjectsAt(from, to, GetGame().GetPlayer(), 0.5 );

		bool selected = false;
		
		for ( int nObject = 0; ( ( nObject < objects.Count() ) && !selected ); ++nObject )
		{
			Object obj = objects.Get( nObject );
			
			SelectObject( obj );
			selected = true;

			ObjectInfoMenu.infoPosX.SetText( m_SelectedObject.GetPosition()[0].ToString() );
			ObjectInfoMenu.infoPosY.SetText( m_SelectedObject.GetPosition()[1].ToString() );
			ObjectInfoMenu.infoPosZ.SetText( m_SelectedObject.GetPosition()[2].ToString() );

			ObjectInfoMenu.infoPosYaw.SetText( m_SelectedObject.GetOrientation()[0].ToString() );
			ObjectInfoMenu.infoPosPitch.SetText( m_SelectedObject.GetOrientation()[1].ToString() );
			ObjectInfoMenu.infoPosRoll.SetText( m_SelectedObject.GetOrientation()[2].ToString() );
		}
	
		if ( !selected && m_SelectedObject )
		{
			GetPlayer().MessageStatus("Current object deselected.");
			DeselectObject();
		}
	}
	
	void DeleteObject() 
	{
		if ( !m_ObjectEditorActive )
		{
			return;
		}

		if ( m_SelectedObject )
		{
			m_Objects.RemoveItem(m_SelectedObject);
			m_SelectedObject.SetPosition(vector.Zero); // If object does not physically delete, teleport it to 0 0 0
			GetGame().ObjectDelete( m_SelectedObject ); 

			ObjectInfoMenu.UpdateObjectList();

			m_SelectedObject = NULL;	
		}
	}
	
	void GroundObject() 
	{
		if ( !m_ObjectEditorActive )
		{
			return;
		}

		if ( m_SelectedObject )
		{
			m_SelectedObject.PlaceOnSurface();
		}
	}
}