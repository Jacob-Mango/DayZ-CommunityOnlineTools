class JMSelectedModule: JMRenderableModuleBase
{
	private autoptr set< Object > m_SelectedObjects;
	
	void JMSelectedModule()
	{
		m_SelectedObjects = new set< Object >;

		GetPermissionsManager().RegisterPermission( "Object.View" );
        
		GetPermissionsManager().RegisterPermission( "Object.Create" );
		GetPermissionsManager().RegisterPermission( "Object.Delete" );
		GetPermissionsManager().RegisterPermission( "Object.Select" );

		g_jm_SelectedModule = this;
	}

	void ~JMSelectedModule()
	{
    }

	override void RegisterKeyMouseBindings() 
	{
	}

	override bool HasAccess()
	{
		return GetPermissionsManager().HasPermission( "Object.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/selected_form.layout";
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
    }

	set< Object > GetObjects()
	{
		return m_SelectedObjects;
	}

	void AddObject( Object obj )
	{
		int idx = m_SelectedObjects.Find( obj );
		if ( idx < 0 )
		{
			m_SelectedObjects.Insert( obj );

			JMSelectedForm frm;
			if ( Class.CastTo( frm, GetForm() ) )
			{
				frm.OnAddObject( obj );
			}
		}
	}

	void RemoveObject( Object obj )
	{
		int idx = m_SelectedObjects.Find( obj );
		if ( idx >= 0 )
		{
			m_SelectedObjects.Remove( idx );

			JMSelectedForm frm;
			if ( Class.CastTo( frm, GetForm() ) )
			{
				frm.OnRemoveObject( obj );
			}
		}
	}

	void ClearObjects()
	{
		m_SelectedObjects.Clear();

		JMSelectedForm frm;
		if ( Class.CastTo( frm, GetForm() ) )
		{
			frm.OnClear();
		}
	}
}

ref JMSelectedModule g_jm_SelectedModule;

ref JMSelectedModule GetObjectSelectedModule()
{
	return g_jm_SelectedModule;
}