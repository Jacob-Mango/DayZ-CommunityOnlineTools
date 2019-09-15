enum JMSelectedModuleRPC
{
    INVALID = 10180,
    Delete,
    COUNT
};

class JMSelectedModule: JMRenderableModuleBase
{
	private autoptr set< Object > m_SelectedObjects;
	
	void JMSelectedModule()
	{
		m_SelectedObjects = new set< Object >;

		GetPermissionsManager().RegisterPermission( "Entity.View" );
		GetPermissionsManager().RegisterPermission( "Entity.Delete" );
		
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
		return GetPermissionsManager().HasPermission( "Entity.View" );
	}

	override string GetLayoutRoot()
	{
		return "JM/COT/GUI/layouts/selected_form.layout";
	}

	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();
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

	int GetRPCMin()
	{
		return JMSelectedModuleRPC.INVALID;
	}

	int GetRPCMax()
	{
		return JMSelectedModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ref ParamsReadContext ctx )
	{
		switch ( rpc_type )
		{
		case JMSelectedModuleRPC.Delete:
			RPC_Delete( ctx, sender, target );
			break;
		}
    }

	void Delete()
	{
		if ( m_SelectedObjects.Count() == 0 )
		{
			return;
		}

		if ( IsMissionClient() )
		{
			if ( !GetPermissionsManager().HasPermission( "Entity.Delete" ) )
			{
				return;
			}

			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( m_SelectedObjects );
			rpc.Send( NULL, JMSelectedModuleRPC.Delete, true, NULL );
		} else
		{
			Server_Delete( m_SelectedObjects, NULL );
		}
	}

	private void Server_Delete( set< Object > objects, PlayerIdentity ident )
	{
		if ( !GetPermissionsManager().HasPermission( "Entity.Delete", ident ) )
		{
			return;
		}

		for ( int i = 0; i < objects.Count(); i++ )
		{
			if ( objects[i] == NULL )
				continue;

			string obtype;
			GetGame().ObjectGetType( objects[i], obtype );

			GetCommunityOnlineToolsBase().Log( ident, "Deleted object " + objects[i].GetDisplayName() + " (" + obtype + ") at " + objects[i].GetPosition() );
			
			GetGame().ObjectDelete( objects[i] );
		}
	}

	private void RPC_Delete( ref ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( IsMissionHost() )
		{
			set< Object > objects;
			if ( !ctx.Read( objects ) )
			{
				Error("Failed");
				return;
			}

			Server_Delete( objects, senderRPC );
		}
	}
}