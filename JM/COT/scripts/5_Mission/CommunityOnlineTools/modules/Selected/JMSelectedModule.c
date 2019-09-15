class JMSelectedModule: JMRenderableModuleBase
{
	private autoptr set< Object > m_SelectedObjects;
	
	void JMSelectedModule()
	{
		m_SelectedObjects = new set< Object >;

		GetRPCManager().AddRPC( "COT_Object", "RPC_Delete", this, SingeplayerExecutionType.Server );

		GetPermissionsManager().RegisterPermission( "Object.View" );
        
		GetPermissionsManager().RegisterPermission( "Object.Create" );
		GetPermissionsManager().RegisterPermission( "Object.Delete" );
		GetPermissionsManager().RegisterPermission( "Object.Select" );
		
		JMScriptInvokers.MENU_OBJECT_BUTTON.Insert( OnObjectButton );
		JMScriptInvokers.MENU_OBJECT_CHECKBOX.Insert( OnObjectCheckbox );
	}

	void ~JMSelectedModule()
	{
		Hide();

		JMScriptInvokers.MENU_OBJECT_BUTTON.Remove( OnObjectButton );
		JMScriptInvokers.MENU_OBJECT_CHECKBOX.Remove( OnObjectCheckbox );
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

	void Delete()
	{
		if ( m_SelectedObjects.Count() > 0 )
		{
			GetRPCManager().SendRPC( "COT_Object", "RPC_Delete", new Param1< ref set< Object > >( m_SelectedObjects ) );
		}
	}

	void RPC_Delete( CallType type, ref ParamsReadContext ctx, ref PlayerIdentity senderRPC, ref Object target )
	{
		if ( !GetPermissionsManager().HasPermission( "Object.Delete", senderRPC ) )
			return;

		ref Param1< ref set< Object > > data;
		if ( !ctx.Read( data ) )
			return;

		set< Object > copy = new set< Object >;
		copy.Copy( data.param1 );
		
		if ( type == CallType.Server )
		{
			for ( int i = 0; i < copy.Count(); i++ )
			{
				if ( copy[i] == NULL )
					continue;

				string obtype;
				GetGame().ObjectGetType( copy[i], obtype );

				GetCommunityOnlineToolsBase().Log( senderRPC, "Deleted object " + copy[i].GetDisplayName() + " (" + obtype + ") at " + copy[i].GetPosition() );
				GetGame().ObjectDelete( copy[i] );
			}
		}
	}

	void OnObjectButton( Object obj, bool check )
	{
		Print( "obj2 " + obj + " will be " + check );
		m_SelectedObjects.Clear();

		if ( !check )
		{
			m_SelectedObjects.Insert( obj );
		}

		JMSelectedForm frm;
		if ( Class.CastTo( frm, GetForm() ) )
		{
			frm.RefreshSelectedList( m_SelectedObjects );
		}
	}

	void OnObjectCheckbox( Object obj, bool checked )
	{
		Print( "obj2 " + obj + " is " + checked );
		if ( checked )
		{
			AddObject( obj );
		} else
		{
			RemoveObject( obj );
		}
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
				frm.RefreshSelectedList( m_SelectedObjects );
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
				frm.RefreshSelectedList( m_SelectedObjects );
			}
		}
	}

	void ClearObjects()
	{
		m_SelectedObjects.Clear();

		JMSelectedForm frm;
		if ( Class.CastTo( frm, GetForm() ) )
		{
			frm.RefreshSelectedList( m_SelectedObjects );
		}
	}
}