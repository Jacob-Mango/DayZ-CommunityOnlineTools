class JMContextMenuEntryData
{
	string MenuType;
	string Id;
	string Label;
	string IconPath;
	int Color;
	bool Submenu;
	Class Target;
	string Callback;

	void JMContextMenuEntryData( string menuType, string id, string label, string icon = "", int color = 0, bool submenu = false, Class target = null, string callback = "" )
	{
		MenuType = menuType;
		Id       = id;
		Label    = label;
		IconPath = icon;
		Color    = color;
		Submenu  = submenu;
		Target   = target;
		Callback = callback;
	}
}

class JMContextMenuRegistry
{
	protected static ref array<ref JMContextMenuEntryData> s_Entries;

	static void Register( string menuType, string id, string label, string icon = "", int color = 0, bool submenu = false, Class target = null, string callback = "" )
	{
		if ( !s_Entries )
			s_Entries = new array<ref JMContextMenuEntryData>;

		s_Entries.Insert( new JMContextMenuEntryData( menuType, id, label, icon, color, submenu, target, callback ) );
	}

	//! Helper for 3D World context actions (auto-applies PREFIX_ACTION "a:")
	static void Register3DWorldAction( string id, string label, string icon = "", Class target = null, string callback = "", int color = 0 )
	{
		string actionId = id;
		if ( actionId.IndexOf( JMESPActionMenu.PREFIX_ACTION ) != 0 && actionId.IndexOf( JMESPActionMenu.PREFIX_PAGE ) != 0 )
			actionId = JMESPActionMenu.PREFIX_ACTION + id;

		Register( "3DWorld", actionId, label, icon, color, false, target, callback );
	}

	static void Populate( string menuType, UIActionContextMenu menu )
	{
		if ( !s_Entries || !menu )
			return;

		foreach ( JMContextMenuEntryData entry : s_Entries )
		{
			if ( entry.MenuType == menuType )
				menu.AddItem( entry.Id, entry.Label, entry.IconPath, entry.Color, entry.Submenu, entry.Target, entry.Callback );
		}
	}
}
