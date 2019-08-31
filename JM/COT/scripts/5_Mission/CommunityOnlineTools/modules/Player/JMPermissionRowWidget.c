class JMPermissionRowWidget extends ScriptedWidgetEventHandler 
{
	private string indent = "";

	ref JMPermissionRowWidget Parent;
	ref array< ref JMPermissionRowWidget > Children;

	string Name;
	int Type;

	protected ref Widget layoutRoot;
	protected ref TStringArray stateOptions;

	ref TextWidget perm_name;
	ref OptionSelectorMultistate perm_state;

	protected bool m_IsEnabled = false;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void ~JMPermissionRowWidget()
	{
		delete Children;
	}

	void Init() 
	{
		Children = new array< ref JMPermissionRowWidget >;

		perm_name = TextWidget.Cast(layoutRoot.FindAnyWidget("permission_name"));

		stateOptions = new TStringArray;
		stateOptions.Insert("INHERIT");
		stateOptions.Insert("DISALLOW");
		stateOptions.Insert("ALLOW");

		perm_state = new OptionSelectorMultistate( layoutRoot.FindAnyWidget( "permission_setting" ), 0, NULL, true, stateOptions );
		perm_state.m_OptionChanged.Insert( OnPermissionStateChanged );

		Disable();
	}

	void Show()
	{
		layoutRoot.Show( true );
		OnShow();
	}

	void Hide()
	{
		OnHide();
		layoutRoot.Show( false );
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	ref Widget GetLayoutRoot() 
	{
		return layoutRoot;
	}

	void Indent( int depth )
	{
		for ( int i = 0; i < depth; i++ )
		{
			indent = "   " + indent;
		}
	}

	void OnPermissionStateChanged()
	{
		Type = perm_state.GetValue();
	}

	void InitPermission( ref JMPermission permission, int depth )
	{
		Indent( depth );

		Name = permission.Name;

		perm_name.SetText( indent + Name );
		perm_state.SetValue( 0, true );
	}

	bool IsEnabled()
	{
		return m_IsEnabled;
	}

	void Enable()
	{
		m_IsEnabled = OnEnable();

		if ( m_IsEnabled )
		{
			for ( int i = 0; i < Children.Count(); i++ )
			{
				Children[i].Enable();
			}
		}
	}

	void Disable()
	{
		m_IsEnabled = !OnDisable();

		if ( !m_IsEnabled )
		{
			for ( int i = 0; i < Children.Count(); i++ )
			{
				Children[i].Disable();
			}
		}
	}

	bool OnEnable()
	{
		perm_state.Enable();

		return true;
	}

	bool OnDisable()
	{
		perm_state.SetValue( 0, true );

		perm_state.Disable();

		return true;
	}

	void Serialize( ref array< string > output, string prepend = "" )
	{
		for ( int i = 0; i < Children.Count(); i++ )
		{
			string serialize = prepend + Children[i].Name;
				
			output.Insert( serialize + " " + Children[i].Type );

			if ( Children[i].Children.Count() > 0 ) 
			{
				Children[i].Serialize( output, serialize + "." );
			}
		}
	}
	
	void SetPermission( string inp )
	{
		array<string> tokens = new array<string>;

		array<string> spaces = new array<string>;
		inp.Split( " ", spaces );

		int type;

		if ( spaces.Count() == 2 )
		{
			if ( spaces[1].Contains( "2" ) )
			{
				type = 2;
			} else if ( spaces[1].Contains( "1" ) )
			{
				type = 1;
			} else 
			{
				type = 0;
			}

			spaces[0].Split( ".", tokens );
		} else if ( spaces.Count() < 2 )
		{
			type = 0;

			inp.Split( ".", tokens );
		} else 
		{
			Print( "Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\"." );
			return;
		}

		//Print( inp + " with type " + type );
		
		int depth = tokens.Find( Name );

		if ( depth > -1 )
		{
			Set( tokens, depth + 1, type );
		} else 
		{
			Set( tokens, 0, type );
		}
	}

	private ref JMPermissionRowWidget Set( array<string> tokens, int depth, int type )
	{
		if ( depth < tokens.Count() )
		{
			for ( int i = 0; i < Children.Count(); i++ )
			{
				if ( Children[i].Name == tokens[depth] )
				{
					return Children[i].Set( tokens, depth + 1, type );
				}
			}
			return NULL;
		}
		
		Enable();
		perm_state.SetValue( type, true );
		return this;
	}
}