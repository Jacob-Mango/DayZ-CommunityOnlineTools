// =============================================================================
//  JMMapEditorCatalog.c
//
//  Static asset catalog for the Map Editor's left sidebar. Modelled after the
//  DayZ Editor's object browser. Every entry is a vanilla classname plus an
//  icon path; categories are collapsible. Custom modded classnames can be
//  added by editing this file (no DB / JSON loader - keep it simple).
//
//  The JMMapEditorAsset data class lives in its own file (one class per file).
//
//  Spawn convention: pick a row -> form enters Spawn mode -> next map click
//  places the object. Cancel spawn by clicking an empty spot or pressing Esc
//  (future hook).
// =============================================================================

class JMMapEditorCatalog
{
	// Icons reused across the catalog (palette matches the rest of COT)
	static const string ICON_DEFAULT = "JM/COT/GUI/textures/icons/cog.edds";

	static ref array< ref JMMapEditorAsset > Build()
	{
		ref array< ref JMMapEditorAsset > items = new array< ref JMMapEditorAsset >;
		ref JMMapEditorAsset e;

		// ---- Vehicles ----
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "OffroadHatchback"; e.DisplayName = "Offroad Hatchback"; e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Sedan";           e.DisplayName = "Sedan";           e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Hatchback";        e.DisplayName = "Hatchback";        e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "CivilianSedan";   e.DisplayName = "Civilian Sedan";   e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Truck_C01";       e.DisplayName = "Truck";            e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Offroad_02";      e.DisplayName = "Offroad (green)";  e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Land_Wreck_Car";  e.DisplayName = "Wrecked Car";      e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Land_Wreck_Truck";e.DisplayName = "Wrecked Truck";    e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "UH1H";            e.DisplayName = "UH-1H Helicopter"; e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Mi8";             e.DisplayName = "Mi-8 Helicopter";  e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Boat_Canoe";      e.DisplayName = "Canoe";            e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Vehicles"; e.ClassName = "Boat_MotorBoat";  e.DisplayName = "Motorboat";        e.IconPath = ICON_DEFAULT; items.Insert( e );

		// ---- Containers / Furniture ----
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "Sea_Chest";      e.DisplayName = "Sea Chest";        e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "WoodCrateKit";    e.DisplayName = "Wood Crate Kit";   e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "Barrel_Green";   e.DisplayName = "Barrel (green)";   e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "Barrel_Red";     e.DisplayName = "Barrel (red)";     e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "Barrel_Blue";    e.DisplayName = "Barrel (blue)";    e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Containers"; e.ClassName = "WoodenCrate";    e.DisplayName = "Wooden Crate";     e.IconPath = ICON_DEFAULT; items.Insert( e );

		// ---- Structures ----
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_Wall_Gate"; e.DisplayName = "Wall Gate";       e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_Wall_Fen";  e.DisplayName = "Wall Fence";      e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_City_Stand";e.DisplayName = "City Stand";      e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_City_Lamp"; e.DisplayName = "City Lamp";       e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_City_TrashCan";e.DisplayName = "Trash Can";    e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Structures"; e.ClassName = "Land_City_ConcreteBarrier_01"; e.DisplayName = "Concrete Barrier"; e.IconPath = ICON_DEFAULT; items.Insert( e );

		// ---- Military ----
		e = new JMMapEditorAsset();
		e.Category = "Military"; e.ClassName = "Land_Wreck_Mi8";        e.DisplayName = "Wrecked Mi-8";        e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Military"; e.ClassName = "Land_Wreck_UH1H";      e.DisplayName = "Wrecked UH-1H";      e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Military"; e.ClassName = "Land_Mil_Barracks1";   e.DisplayName = "Military Barracks";   e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Military"; e.ClassName = "Land_Mil_Radar_Small";  e.DisplayName = "Radar (small)";      e.IconPath = ICON_DEFAULT; items.Insert( e );

		// ---- Nature ----
		e = new JMMapEditorAsset();
		e.Category = "Nature"; e.ClassName = "Land_Camp_Fire";  e.DisplayName = "Campfire";    e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Nature"; e.ClassName = "bldr_tree_birch";e.DisplayName = "Birch Tree";  e.IconPath = ICON_DEFAULT; items.Insert( e );
		e = new JMMapEditorAsset();
		e.Category = "Nature"; e.ClassName = "bldr_tree_oak";  e.DisplayName = "Oak Tree";    e.IconPath = ICON_DEFAULT; items.Insert( e );

		return items;
	}

	// Returns a sorted list of unique categories (Vehicles, Containers, ...)
	static array< string > GetCategories()
	{
		ref array< ref JMMapEditorAsset > items = Build();
		ref array< string > cats = new array< string >;
		for ( int i = 0; i < items.Count(); i++ )
		{
			string c = items.Get( i ).Category;
			bool found = false;
			for ( int j = 0; j < cats.Count(); j++ )
			{
				if ( cats.Get( j ) == c )
				{
					found = true;
					break;
				}
			}
			if ( !found )
				cats.Insert( c );
		}
		return cats;
	}
}