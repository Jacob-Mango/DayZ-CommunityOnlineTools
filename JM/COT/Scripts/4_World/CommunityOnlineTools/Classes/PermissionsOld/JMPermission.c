#ifndef CF_MODULE_PERMISSIONS
class JMPermission : Managed
{
	JMPermission Root;
	JMPermission Parent;

	ref array< ref JMPermission > Children;

	string Name;

	private string m_SerializedFullName;

	JMPermissionType Type;

	Widget View;

	string Indent;

	bool m_Sync;  //! This will be set to true if any added permission is not INHERIT
	bool m_Sorted;

	void JMPermission( string name, JMPermission parent = NULL )
	{
		Name = name;
		Parent = parent;
		if (!parent)
			Root = this;
		else
			Root = parent.Root;

		Type = JMPermissionType.INHERIT;

		if ( Parent == NULL )
		{
			Type = JMPermissionType.DISALLOW;
		}

		Children = new array< ref JMPermission >;

		UpdateFullName();
	}

	void CopyPermissions(JMPermission from)
	{
		Children.Clear();
		foreach (auto child: from.Children)
		{
			auto perm = new JMPermission(child.Name, this);
			#ifdef COT_DEBUGLOGS
			CF_Log.Debug("Copying %1 %2", child.GetFullName(), child.Type.ToString());
			#endif
			perm.Type = child.Type;
			perm.CopyPermissions(child);
			Children.Insert(perm);
			if (child.Type != JMPermissionType.INHERIT)
				Root.m_Sync = true;
		}
	}

	void UpdateFullName()
	{
		m_SerializedFullName = Name;

		JMPermission parent = Parent;
		while ( parent != NULL )
		{
			if ( parent.Parent == NULL ) 
			{
				return;
			}
			
			m_SerializedFullName = parent.Name + "." +  m_SerializedFullName;
			parent = parent.Parent;
		}
	}

	string GetFullName()
	{
		return m_SerializedFullName;
	}

	void AddPermission( string inp, JMPermissionType permType = JMPermissionType.INHERIT, bool requireRegistered = true )
	{
		array<string> tokens = new array<string>;

		array<string> spaces = new array<string>;
		inp.Split( " ", spaces );

		JMPermissionType type;

		if ( permType == JMPermissionType.INHERIT && spaces.Count() == 2 )
		{
			if ( spaces[1].Contains( "2" ) )
			{
				type = JMPermissionType.ALLOW;
			} else if ( spaces[1].Contains( "1" ) )
			{
				type = JMPermissionType.DISALLOW;
			} else 
			{
				type = JMPermissionType.INHERIT;
			}

			spaces[0].Split( ".", tokens );
		} else if ( spaces.Count() < 2 )
		{
			type = permType;

			inp.Split( ".", tokens );
		} else {
			Error( "Warning, permission line improperly formatted! Read as \"" + inp + "\" but meant to be in format \"Perm.Perm {n}\"." );
			return;
		}
		
		int depth = tokens.Find( Name );

		if ( depth > -1 )
		{
			AddPermissionInternal( tokens, depth + 1, type, requireRegistered );
		} else 
		{
			AddPermissionInternal( tokens, 0, type, requireRegistered );
		}
	}

	private void AddPermissionInternal( array<string> tokens, int depth, JMPermissionType value, bool requireRegistered = true )
	{
		if ( depth < tokens.Count() )
		{
			string name = tokens[depth];

			//Print(ClassName() + "::AddPermissionInternal " + m_SerializedFullName + "." + name);

			JMPermission nChild = VerifyAddPermission( name, requireRegistered );
			if (nChild)
				nChild.AddPermissionInternal( tokens, depth + 1, value, requireRegistered );
		} else {
			Type = value;
			if (value != JMPermissionType.INHERIT)
				Root.m_Sync = true;
		}
	}

	private JMPermission VerifyAddPermission( string name, bool requireRegistered = false )
	{
		JMPermission nChild = NULL;

		for ( int i = 0; i < Children.Count(); i++ )
		{
			if ( name == Children[i].Name )
			{
				nChild = Children[i];
				break;
			}
		}
			
		if ( !requireRegistered && nChild == NULL )
		{
			nChild = new JMPermission( name, this );
			nChild.Type = JMPermissionType.INHERIT;

			Children.Insert( nChild );
		}

		return nChild;
	}

	JMPermission GetPermission( string inp )
	{
		array<string> tokens = new array<string>;
		inp.Split( ".", tokens );
		
		int depth = tokens.Find(Name);

		if ( depth > -1 )
		{
			return Get( tokens, depth + 1 );
		} else 
		{
			return Get( tokens, 0 );
		}
	}

	private JMPermission Get( array<string> tokens, int depth )
	{
		if ( depth < tokens.Count() )
		{
			JMPermission nChild = NULL;

			for ( int i = 0; i < Children.Count(); i++ )
			{
				if ( Children[i].Name == tokens[depth] )
				{
					nChild = Children[i]; 
				}
			}

			if ( nChild )
			{
				return nChild.Get( tokens, depth + 1 );
			}
		}

		return this;
	}

	bool HasPermission( string inp, out JMPermissionType permType )
	{
		array<string> tokens = new array<string>;
		inp.Split( ".", tokens );

		if ( tokens.Count() == 0 )
			return false;
		
		int depth = tokens.Find( Name );

		bool parentDisallowed = false;

		if ( Type == JMPermissionType.DISALLOW )
		{
			parentDisallowed = true;
		} else if ( Type == JMPermissionType.INHERIT )
		{
			JMPermission parent = Parent;
			while ( parent != NULL )
			{
				if ( parent.Type != JMPermissionType.INHERIT )
				{
					if ( parent.Type == JMPermissionType.DISALLOW )
					{
						parentDisallowed = true;
					}

					break;
				}
				
				parent = parent.Parent;
			}
		}
		
		if ( depth > -1 )
		{
			return Check( tokens, depth + 1, parentDisallowed, permType );
		} else 
		{
			return Check( tokens, 0, parentDisallowed, permType );
		}
	}

	bool Check( array<string> tokens, int depth, bool parentDisallowed, out JMPermissionType permType )
	{
		bool ifReturnAs = false;

		if ( Type == JMPermissionType.ALLOW )
		{
			ifReturnAs = true;
		}

		if ( Type == JMPermissionType.INHERIT && parentDisallowed == false )
		{
			ifReturnAs = true;
		}

		if ( Type == JMPermissionType.DISALLOW )
		{
			parentDisallowed = true;
		}

		if ( Type == JMPermissionType.ALLOW )
		{
			parentDisallowed = false;
		}

		if ( depth < tokens.Count() )
		{
			JMPermission nChild = NULL;

			for ( int i = 0; i < Children.Count(); i++ )
			{
				if ( Children[i].Name == tokens[depth] )
				{
					nChild = Children[i]; 
				}
			}

			if ( nChild )
			{
				return nChild.Check( tokens, depth + 1, parentDisallowed, permType );
			}
		}

		permType = Type;

		return ifReturnAs;
	}

	void Clear()
	{
		Children.Clear();
	}

	void Serialize( array< string > output, string prepend = "" )
	{
		if (!Parent)
			auto trace = CF_Trace_2(this, "Serialize").Add(m_SerializedFullName).Add(Children.Count());

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

	void Deserialize( array< string > input )
	{
		if (!Parent)
			auto trace = CF_Trace_1(this, "Deserialize").Add(input.Count());

		for ( int i = 0; i < input.Count(); i++ )
			AddPermission( input[i], JMPermissionType.INHERIT );
	}

	void OnSend(ParamsWriteContext ctx)
	{
		if (!m_Sorted)
			Sort();

		if (!Root.m_Sync)
		{
			ctx.Write(-1);
			return;
		}

		ctx.Write(Children.Count());

		foreach (auto child: Children)
		{
			#ifdef COT_DEBUGLOGS
			CF_Log.Debug("Sending %1 %2", child.GetFullName(), child.Type.ToString());
			#endif
			ctx.Write(child.Type);
			child.OnSend(ctx);
		}
	}

	bool OnReceive(ParamsReadContext ctx)
	{
		if (!m_Sorted)
			Sort();

		int count;
		if (!ctx.Read(count))
			return false;

		if (count == -1)
			return true;

		if (count != Children.Count())
			Error(string.Format("Received child count %1 for %2 does not match registered child count %3!", count, m_SerializedFullName, Children.Count()));

		foreach (auto child: Children)
		{
			int type;
			if (!ctx.Read(type))
				return false;

			child.Type = type;
			#ifdef COT_DEBUGLOGS
			CF_Log.Debug("Received %1 %2", child.GetFullName(), child.Type.ToString());
			#endif
			if (!child.OnReceive(ctx))
				return false;
		}

		return true;
	}

	void DebugPrint( int depth = 0 )
	{
		string message = this.Name;

		for ( int j = 0; j < depth; j++ )
		{
			message = "  " + message;
		}

		string append = "";

		switch ( Type )
		{
			case JMPermissionType.ALLOW:
			{
				append = " ALLOW";
				break;
			}
			case JMPermissionType.DISALLOW:
			{
				append = " DISALLOW";
				break;
			}
			default:
			case JMPermissionType.INHERIT:
			{
				append = " INHERIT";
				break;
			}
		}

		// Print( message + append );

		for ( int i = 0; i < Children.Count(); i++ )
		{
			Children[i].DebugPrint( depth + 1 );
		}
	}

	void Sort()
	{
	#ifdef DIAG
		CF_Trace trace;
		if (!Parent)
			trace = CF_Trace_2(this).Add(GetFullName()).Add(typename.EnumToString(JMPermissionType, Type));
	#endif

		m_Sorted = true;

		if (Children.Count() < 2)
			return;

		TStringArray names = {};
		map<string, ref JMPermission> permissions = new map<string, ref JMPermission>;

		foreach (int i, JMPermission child: Children)
		{
			names.Insert(child.Name);
			permissions.Insert(child.Name, child);
		//#ifdef DIAG
			//CF_Log.Trace(" IN  %1 %2 %3", i.ToString(), child.GetFullName(), typename.EnumToString(JMPermissionType, child.Type));
		//#endif
		}

		names.Sort();

		Children.Clear();

		JMPermission permission;
		foreach (int j, string name: names)
		{
			permission = permissions[name];
			Children.Insert(permission);
		//#ifdef DIAG
			//CF_Log.Trace(" OUT %1 %2 %3", j.ToString(), permission.GetFullName(), typename.EnumToString(JMPermissionType, permission.Type));
		//#endif
		}
	}
};
#endif