class JMPermissionRowWidget extends ScriptedWidgetEventHandler 
{
	private string indent = "";

	JMPermissionRowWidget Parent;
	ref array< JMPermissionRowWidget > Children;

	string Name;
	string FullName;
	int Type;

	private Widget layoutRoot;
	private ref TStringArray stateOptions;

	private TextWidget m_txt_PermissionName;
	private ref OptionSelectorMultistate m_state_Permission;

	private bool m_IsEnabled = false;

	void OnWidgetScriptInit( Widget w )
	{
		layoutRoot = w;
		layoutRoot.SetHandler( this );

		Init();
	}

	void JMPermissionRowWidget()
	{
		Children = new array< JMPermissionRowWidget >;

		stateOptions = new TStringArray;
		stateOptions.Insert( "#STR_COT_PERMISSION_INHERIT_C" );
		stateOptions.Insert( "#STR_COT_PERMISSION_DISALLOW_C" );
		stateOptions.Insert( "#STR_COT_PERMISSION_ALLOW_C" );
	}

	void ~JMPermissionRowWidget()
	{
		delete Children;
		delete stateOptions;
	}

	void Init() 
	{
		Class.CastTo( m_txt_PermissionName, layoutRoot.FindAnyWidget( "permission_name" ) );

		m_state_Permission = new OptionSelectorMultistate( layoutRoot.FindAnyWidget( "permission_setting" ), 0, NULL, true, stateOptions );
		m_state_Permission.m_OptionChanged.Insert( OnPermissionStateChanged );

		Disable();
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		layoutRoot.Show( true );
		OnShow();

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif

		OnHide();
		layoutRoot.Show( false );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void OnShow()
	{
	}

	void OnHide() 
	{
	}

	Widget GetLayoutRoot() 
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
		Type = m_state_Permission.GetValue();
	}

	void InitPermission( JMPermission permission )
	{
		Indent( permission.Depth );

		Name = permission.Name;
		FullName = permission.GetFullName();

		permission.View = layoutRoot;

		m_txt_PermissionName.SetText( indent + Name );
		m_state_Permission.SetValue( 0, true );
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
		m_state_Permission.Enable();

		return true;
	}

	bool OnDisable()
	{
		m_state_Permission.SetValue( 0, true );

		m_state_Permission.Disable();

		return true;
	}

	void Serialize( out array< string > output, string prepend = "" )
	{
		auto trace = CF_Trace_0(this, "Serialize");

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
			#ifdef JM_COT_LOGGING
			Print( "Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\"." );
			#endif
			return;
		}
		
		#ifdef JM_COT_LOGGING
		Print( inp + " with type " + type );
		#endif
		
		int depth = tokens.Find( Name );

		if ( depth > -1 )
		{
			Set( tokens, depth + 1, type );
		} else 
		{
			Set( tokens, 0, type );
		}
	}

	private JMPermissionRowWidget Set( array<string> tokens, int depth, int type )
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
		m_state_Permission.SetValue( type, true );
		return this;
	}

	void SetType( JMPermissionType type )
	{
		m_state_Permission.SetValue( type, true );
	}
}