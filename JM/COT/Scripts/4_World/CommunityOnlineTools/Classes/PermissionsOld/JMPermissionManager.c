#ifndef CF_MODULE_PERMISSIONS
class JMPermissionManager
{
	ref map< string, ref JMPlayerInstance > Players;
	ref map< string, ref JMRole > Roles;

	ref map< string, string > SteamToGUID;

	ref JMPermission RootPermission;

	private string m_ClientGUID;

	//! protected, not private: DayZ-Expansion's `modded class JMPermissionManager`
	//! reads this in its Expansion_RegisterPermission override. A modded class
	//! cannot touch a private member of the class it mods, so privatising this
	//! breaks Expansion's compile. Keep protected for mod compatibility.
	protected bool m_MissionLoaded;

	void JMPermissionManager()
	{
		Players = new map< string, ref JMPlayerInstance >;
		Roles = new map< string, ref JMRole >;

		SteamToGUID = new map< string, string >;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		
		if ( IsMissionHost() )
		{
			MakeDirectory( JMConstants.DIR_PF );

			MakeDirectory( JMConstants.DIR_PERMISSIONS );
			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
		}
		
		Assert_Null( Players );
		Assert_Null( Roles );
		Assert_Null( SteamToGUID );
		Assert_Null( RootPermission );
	}

	void GetRolesAsList( out array< JMRole > roles )
	{
		Assert_Null( Roles );

		if ( roles == NULL )
			roles = new array< JMRole >();

		roles.Copy( Roles.GetValueArray() );

		roles.Debug();
	}

	void GetPermissionsAsList( out array< JMPermission > permissions )
	{
		Assert_Null( RootPermission );

		if ( permissions == NULL )
			permissions = new array< JMPermission >();

		GetPermissionsAsList( RootPermission, "", permissions );
	}

	void GetPermissionsAsList( JMPermission permission, out array< JMPermission > permissions )
	{
		Assert_Null( RootPermission );

		if ( permissions == NULL )
			permissions = new array< JMPermission >();

		GetPermissionsAsList( permission, "", permissions );
	}

	private void GetPermissionsAsList( JMPermission permission, string indent, inout array< JMPermission > permissions, bool last = false )
	{
		Assert_Null( RootPermission );
		Assert_Null( permission );

		if (permission.Parent && permission.Parent.Name != "ROOT")
		{
			if (last)
			{
				permission.Indent = indent + " \\- ";
				indent += "    ";
			}
			else
			{
				permission.Indent = indent + " |- ";
				indent += " |  ";
			}
		}

		for ( int i = 0; i < permission.Children.Count(); ++i )
		{
			permissions.Insert( permission.Children[i] );

			GetPermissionsAsList( permission.Children[i], indent, permissions, i == permission.Children.Count() - 1 );
		}
	}

	string GetClientGUID()
	{
		return m_ClientGUID;
	}

	void SetClientGUID( string guid )
	{
		m_ClientGUID = guid;
	}

	JMPlayerInstance GetClientPlayer()
	{
		return Players.Get( m_ClientGUID );
	}

	void ResetMission()
	{
		Assert_Null( Players );
		Assert_Null( Roles );

		if ( !IsMissionClient() )
			return;

		Players.Clear();
		Roles.Clear();
	}

	void CreateFakePlayers()
	{
		vector basePos = "7500 0 7500".ToVector();
		if ( g_Game.GetPlayer() )
		{
			basePos = g_Game.GetPlayer().GetPosition();
			if ( basePos == vector.Zero )
				basePos = "7500 0 7500".ToVector();
		}

		for ( int i = 0; i < 30; i++ )
		{
			CreateFakePlayer( i, basePos );
		}
	}

	private EntityAI CreateItemWithRandomHealth( EntityAI parent, string classname )
	{
		if ( !parent || classname == "" )
			return NULL;

		EntityAI item = EntityAI.Cast( parent.GetInventory().CreateAttachment( classname ) );
		if ( !item )
			item = EntityAI.Cast( parent.GetInventory().CreateInInventory( classname ) );
		if ( item )
		{
			float health01 = Math.RandomFloat( 0.25, 1.0 );
			item.SetHealth01( "", "Health", health01 );

			Magazine mag = Magazine.Cast( item );
			if ( mag )
			{
				int maxAmmo = mag.GetAmmoMax();
				if ( maxAmmo > 1 )
				{
					int randAmmo = Math.RandomIntInclusive( 1, maxAmmo );
					mag.ServerSetAmmoCount( randAmmo );
				}
			}
			else
			{
				ItemBase itemBase = ItemBase.Cast( item );
				if ( itemBase )
				{
					float maxQty = itemBase.GetQuantityMax();
					if ( maxQty > 1 )
					{
						float randQty = Math.RandomFloat( 1.0, maxQty );
						itemBase.SetQuantity( randQty );
					}
				}
			}
		}
		return item;
	}

	private void CreateWeaponForSurvivor( PlayerBase survivor )
	{
		array<string> firearms = { "M4A1", "AKM", "MakarovIJ70", "FNX45", "MP5K", "Mosina", "SVD", "UMP45" };
		array<string> meleeWeapons = { "FirefighterAxe", "Hatchet", "Machete", "BaseballBat", "Pitchfork", "HuntingKnife", "Crowbar" };

		if ( Math.RandomFloat( 0.0, 1.0 ) > 0.2 )
		{
			if ( Math.RandomFloat( 0.0, 1.0 ) > 0.35 )
			{
				string firearmClass = firearms.GetRandomElement();
				EntityAI wpn = CreateItemWithRandomHealth( survivor, firearmClass );
				if ( wpn )
				{
					if ( firearmClass == "M4A1" )
					{
						CreateItemWithRandomHealth( wpn, "M4_RISHndgrd_Black" );
						CreateItemWithRandomHealth( wpn, "M4_MPBttstck_Black" );
						CreateItemWithRandomHealth( wpn, "ACOGOptic" );
						CreateItemWithRandomHealth( wpn, "M4_Suppressor" );
						CreateItemWithRandomHealth( wpn, "Mag_STANAG_30Rnd" );
					}
					else if ( firearmClass == "AKM" )
					{
						CreateItemWithRandomHealth( wpn, "AK_WoodHndgrd" );
						CreateItemWithRandomHealth( wpn, "AK_WoodBttstck" );
						CreateItemWithRandomHealth( wpn, "KobraOptic" );
						CreateItemWithRandomHealth( wpn, "AK_Suppressor" );
						CreateItemWithRandomHealth( wpn, "Mag_AKM_30Rnd" );
					}
					else if ( firearmClass == "MakarovIJ70" )
					{
						CreateItemWithRandomHealth( wpn, "PistolSuppressor" );
						CreateItemWithRandomHealth( wpn, "Mag_IJ70_8Rnd" );
					}
					else if ( firearmClass == "FNX45" )
					{
						CreateItemWithRandomHealth( wpn, "PistolSuppressor" );
						EntityAI opticFnx = CreateItemWithRandomHealth( wpn, "ReflexOptic" );
						if ( opticFnx )
							CreateItemWithRandomHealth( opticFnx, "Battery9V" );
						CreateItemWithRandomHealth( wpn, "Mag_FNX45_15Rnd" );
					}
					else if ( firearmClass == "MP5K" )
					{
						CreateItemWithRandomHealth( wpn, "MP5_RailHndgrd" );
						CreateItemWithRandomHealth( wpn, "MP5k_StockBttstck" );
						CreateItemWithRandomHealth( wpn, "PistolSuppressor" );
						CreateItemWithRandomHealth( wpn, "Mag_MP5_30Rnd" );
					}
					else if ( firearmClass == "Mosina" )
					{
						CreateItemWithRandomHealth( wpn, "PUScopeOptic" );
					}
					else if ( firearmClass == "SVD" )
					{
						CreateItemWithRandomHealth( wpn, "PSO1Optic" );
						CreateItemWithRandomHealth( wpn, "Mag_SVD_10Rnd" );
					}
					else if ( firearmClass == "UMP45" )
					{
						CreateItemWithRandomHealth( wpn, "PistolSuppressor" );
						EntityAI opticUmp = CreateItemWithRandomHealth( wpn, "ReflexOptic" );
						if ( opticUmp )
							CreateItemWithRandomHealth( opticUmp, "Battery9V" );
						CreateItemWithRandomHealth( wpn, "Mag_UMP_25Rnd" );
					}
				}
			}
			else
			{
				string meleeClass = meleeWeapons.GetRandomElement();
				CreateItemWithRandomHealth( survivor, meleeClass );
			}
		}
	}

	private void CreateFakePlayer( int i, vector basePos )
	{
		array<string> firstNames = { "Adam", "Alex", "Alice", "Arthur", "Ben", "Charlie", "Chloe", "Daniel", "David", "Emma", "Ethan", "Fiona", "George", "Hannah", "Ian", "Jack", "Julia", "Kevin", "Laura", "Liam", "Lucas", "Marcus", "Mia", "Noah", "Oliver", "Rachel", "Sam", "Sophie", "Victor", "Zack" };
		array<string> lastNames = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Wilson", "Taylor", "Anderson", "Thomas", "White", "Harris", "Martin", "Thompson", "Garcia", "Martinez", "Robinson", "Clark", "Rodriguez", "Lewis", "Lee", "Walker", "Hall", "Allen", "Young", "Hernandez", "King", "Wright", "Lopez" };

		string firstName = firstNames.GetRandomElement();
		string lastName = lastNames.GetRandomElement();
		string fakeName = firstName + " " + lastName;

		string fakeGuid = "GFake_" + i + "_" + Math.RandomInt( 1000, 9999 );
		string fakeSteam = "76561198" + ( 70000000 + i * 1337 );

		JMPlayerInstance instance = new JMPlayerInstance( NULL );
		instance.MakeFake( fakeGuid, fakeSteam, fakeName );

		array<string> survivorClasses = { "SurvivorM_Mirek", "SurvivorM_Denis", "SurvivorM_Boris", "SurvivorM_Cyril", "SurvivorM_Elias", "SurvivorM_Francis", "SurvivorM_Guilherme", "SurvivorM_Hassan", "SurvivorM_Indar", "SurvivorM_Jiri", "SurvivorM_Miro", "SurvivorM_Peter", "SurvivorM_Quinn", "SurvivorM_Seth", "SurvivorM_Taiki", "SurvivorF_Eva", "SurvivorF_Frida", "SurvivorF_Gabi", "SurvivorF_Helga", "SurvivorF_Irena", "SurvivorF_Judy", "SurvivorF_Keiko", "SurvivorF_Linda", "SurvivorF_Maria", "SurvivorF_Naomi" };
		string survivorClass = survivorClasses.GetRandomElement();

		float angle = ( 360.0 / 30.0 ) * i + Math.RandomFloat( -5.0, 5.0 );
		float dist = Math.RandomFloat( 10.0, 80.0 );
		float offsetX = Math.Cos( angle * Math.DEG2RAD ) * dist;
		float offsetZ = Math.Sin( angle * Math.DEG2RAD ) * dist;
		float posX = basePos[0] + offsetX;
		float posZ = basePos[2] + offsetZ;
		float posY = g_Game.SurfaceY( posX, posZ );
		vector spawnPos = Vector( posX, posY, posZ );

		PlayerBase survivor = PlayerBase.Cast( GetGame().CreateObjectEx( survivorClass, spawnPos, ECE_PLACE_ON_SURFACE ) );
		if ( survivor )
		{
			survivor.SetOrientation( Vector( Math.RandomFloat( 0.0, 360.0 ), 0, 0 ) );

			float randHealth = Math.RandomFloat( 20.0, 100.0 );
			float randBlood = Math.RandomFloat( 2500.0, 5000.0 );
			float randShock = Math.RandomFloat( 50.0, 100.0 );
			survivor.SetHealth( "GlobalHealth", "Health", randHealth );
			survivor.SetHealth( "GlobalHealth", "Blood", randBlood );
			survivor.SetHealth( "GlobalHealth", "Shock", randShock );

			if ( survivor.GetStatEnergy() )
				survivor.GetStatEnergy().Set( Math.RandomFloat( 300.0, 5000.0 ) );
			if ( survivor.GetStatWater() )
				survivor.GetStatWater().Set( Math.RandomFloat( 300.0, 5000.0 ) );
			if ( survivor.GetStatStamina() )
				survivor.GetStatStamina().Set( Math.RandomFloat( 20.0, 100.0 ) );
			if ( survivor.GetStatHeatBuffer() )
				survivor.GetStatHeatBuffer().Set( Math.RandomFloat( 0.0, 30.0 ) );
			if ( survivor.GetStatWet() )
				survivor.GetStatWet().Set( Math.RandomFloat( 0.0, 0.8 ) );
			if ( survivor.GetStatBloodType() )
				survivor.GetStatBloodType().Set( Math.RandomInt( 0, 8 ) );

			if ( Math.RandomFloat( 0.0, 1.0 ) <= 0.30 )
			{
				int ailmentType = Math.RandomIntInclusive( 0, 2 );
				if ( ailmentType == 0 || ailmentType == 2 )
				{
					survivor.GetModifiersManager().ActivateModifier( eModifiers.MDF_BROKEN_LEGS );
					survivor.SetBrokenLegs( eBrokenLegs.BROKEN_LEGS );
				}
				if ( ailmentType == 1 || ailmentType == 2 )
				{
					array<int> diseases = { eModifiers.MDF_CHOLERA, eModifiers.MDF_INFLUENZA, eModifiers.MDF_SALMONELLA, eModifiers.MDF_POISONING, eModifiers.MDF_WOUND_INFECTION1 };
					int diseaseMod = diseases.GetRandomElement();
					survivor.GetModifiersManager().ActivateModifier( diseaseMod );
				}
			}

			array<string> tops = { "TShirt_Black", "TShirt_Beige", "TShirt_Blue", "Shirt_CheckRed", "Shirt_PlaneBlack", "M65Jacket_Black", "M65Jacket_Khaki", "TTsKOJacket_Camo", "GorkaEJacket_Puch", "HikingJacket_Blue", "Hoodie_Black", "Hoodie_Grey" };
			array<string> pantsPool = { "CargoPants_Black", "CargoPants_Beige", "Jeans_Blue", "Jeans_Black", "TTSKOPants", "HunterPants_Autumn", "TracksuitPants_Black" };
			array<string> shoesPool = { "AthleticShoes_Black", "HikingBoots_Brown", "MilitaryBoots_Black", "CombatBoots_Black", "WorkingBoots_Beige" };
			array<string> hatsPool = { "BaseballCap_Black", "BeanieHat_Black", "MilitaryBeret_UN", "Bandana_Black", "Mich2001Helmet", "CowboyHat_Brown" };
			array<string> vestsPool = { "ChestHolster", "HighCapacityVest_Black", "PlateCarrierHolster", "TacticalVest", "MilitaryBelt" };
			array<string> bagsPool = { "TortillaBag", "HuntingBag", "AliceBag_Camo", "MountainBag_Blue", "DryBag_Orange", "SchoolBag_Blue" };
			array<string> lootPool = { "BandageDressing", "Morphine", "Epinephrine", "TetracyclineAntibiotics", "SalineBagIV", "FirstAidKit", "Apple", "Peach", "CanOpener", "TunaCan", "BakedBeansCan", "SodaCan_Cola", "SodaCan_Spite", "Canteen", "WaterBottle", "Flashlight", "Matchbox", "Compass", "Screwdriver", "Pliers", "Hammer", "Rope", "Headtorch_Grey", "Radio", "Ammo_556x45", "Ammo_762x39", "Ammo_9x19", "Ammo_45ACP" };

			CreateItemWithRandomHealth( survivor, tops.GetRandomElement() );
			CreateItemWithRandomHealth( survivor, pantsPool.GetRandomElement() );
			CreateItemWithRandomHealth( survivor, shoesPool.GetRandomElement() );

			if ( Math.RandomFloat( 0.0, 1.0 ) > 0.3 )
				CreateItemWithRandomHealth( survivor, hatsPool.GetRandomElement() );
			if ( Math.RandomFloat( 0.0, 1.0 ) > 0.4 )
				CreateItemWithRandomHealth( survivor, vestsPool.GetRandomElement() );
			if ( Math.RandomFloat( 0.0, 1.0 ) > 0.2 )
				CreateItemWithRandomHealth( survivor, bagsPool.GetRandomElement() );

			CreateWeaponForSurvivor( survivor );

			int numLoot = Math.RandomInt( 3, 7 );
			for ( int j = 0; j < numLoot; j++ )
			{
				CreateItemWithRandomHealth( survivor, lootPool.GetRandomElement() );
			}

			instance.PlayerObject = survivor;
			instance.Update();
		}

		Players.Insert( instance.GetGUID(), instance );
	}

	/**
	 * This uses GUIDs now.
	 */
	array< JMPlayerInstance > GetPlayers( array< string > guidsGetPlayers = NULL )
	{
		Assert_Null( Players );
		
		if ( guidsGetPlayers == NULL || !g_Game.IsMultiplayer() )
			return Players.GetValueArray();

		array< JMPlayerInstance > players = new array< JMPlayerInstance >;

		for ( int i = 0; i < guidsGetPlayers.Count(); i++ )
		{
			JMPlayerInstance instance = Players.Get( guidsGetPlayers[i] );
			if ( instance )
				players.Insert( instance );
		}

		return players;
	}

	void RegisterPermission( string permission )
	{
		RegisterPermission( permission, JMPermissionType.INHERIT );
	}

	//! Overload taking an explicit permission type.
	//!
	//! Mod-compat: DayZ-Expansion carries its own `Expansion_RegisterPermission`
	//! in a `modded class JMPermissionManager` that is a verbatim copy of this
	//! body, duplicated only so it could pass a permType. Exposing the overload
	//! here means Expansion (and any other mod) can call
	//! `RegisterPermission(name, type)` directly instead of reimplementing it
	//! against COT internals. Expansion's own copy keeps working either way.
	void RegisterPermission( string permission, JMPermissionType permType )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_1(this, "RegisterPermission").Add(permission);
		#endif

		Assert_Null( RootPermission );

		if (m_MissionLoaded)
			Error("Cannot register new permissions once mission is loaded!");
		else
			RootPermission.AddPermission( permission, permType, false );
	}

	//! Mod-compat: DayZ-Expansion calls GetPermissionsManager().IsAdminToolsToggledOn()
	//! from ~25 sites across BaseBuilding / Core (recipes, user actions, territory).
	//! Expansion supplies this via `modded class JMPermissionManager` when JM_COT is
	//! defined, but defining it here means the symbol resolves even if Expansion's
	//! modded class is absent, load-ordered differently, or its COT block is compiled
	//! out. Expansion's modded override returns the same value, so behaviour is
	//! identical whichever definition wins.
	bool IsAdminToolsToggledOn()
	{
		return GetCommunityOnlineToolsBase().IsActive();
	}

	array< string > Serialize()
	{
		auto trace = CF_Trace_1(this, "Serialize").Add(RootPermission.Children.Count());

		Assert_Null( RootPermission );

		array< string > data = new array< string >;
		RootPermission.Serialize( data );
		return data;
	}

	JMPermission GetRootPermission()
	{
		Assert_Null( RootPermission );

		return RootPermission;
	}

	bool HasPermission( string permission, out JMPlayerInstance instance = null )
	{
		if ( IsMissionClient() ) 
		{
			instance = GetClientPlayer();

			if ( IsMissionHost() )
				return true;
			
			if ( !instance /*Assert_Null( instance )*/ )
				return false;

			return instance.HasPermission( permission );
		}

		Error( "JMPermissionManager::HasPermission( permission = " + permission + " ) bool; was called on server!" );

		return false;
	}

	bool HasPermission( string permission, PlayerIdentity ihp )
	{
		if ( IsMissionOffline() )
			return true;

		JMPlayerInstance instance;
		return HasPermission( permission, ihp, instance );
	}

	bool HasPermission( string permission, PlayerIdentity identity, out JMPlayerInstance instance )
	{		
		if ( IsMissionClient() ) 
		{
			instance = GetClientPlayer();

			if ( IsMissionHost() )
				return true;
			else if ( Assert_Null( instance ) )
				return false;

			return instance.HasPermission( permission );
		}

		instance = Players.Get( identity.GetId() );
		if ( Assert_Null( instance ) )
		{
			Print( "JMPlayerInstance does not exist for " + identity.GetId() );
			return false;
		}

		bool allowed = instance.HasPermission( permission );

		//! This overload is the single choke point every incoming COT RPC goes
		//! through on the server, and a refusal here means a client asked for
		//! something its own UI never offers it. That is close to the strongest
		//! signal available from inside a mod: a legitimate client cannot
		//! produce it by accident. Reported, not acted on - the anti-cheat
		//! module decides whether a burst of these is worth a flag.
		if ( !allowed )
			JMAntiCheatSignals.ReportDeniedRpc( identity.GetId(), permission );

		return allowed;
	}

	bool HasPermissions( TStringArray permissions, PlayerIdentity identity, out JMPlayerInstance instance, bool requireAll = true )
	{
		foreach (string permission: permissions)
		{
			if (HasPermission(permission, identity, instance))
			{
				if (!requireAll)
					return true;
			}
			else if (requireAll)
			{
				return false;
			}
		}

		return true;
	}

	bool HasQuickActionAccess(PlayerBase player)
	{
	#ifdef SERVER
		PlayerIdentity identity = player.GetIdentity();
		if (identity && GetCommunityOnlineToolsBase().IsActive(identity) && HasPermission("Actions.QuickActions", identity))
	#else
		if (GetCommunityOnlineToolsBase().IsActive() && HasPermission("Actions.QuickActions"))
	#endif
			return true;

		return false;
	}

	bool OnClientConnected( PlayerIdentity ident, out JMPlayerInstance inst )
	{
		Assert_Null( RootPermission );
		Assert_Null( Players );

		string guid = "";
		
		if ( ident == NULL )
		{
			if ( g_Game.IsMultiplayer() )
				return false;

			guid = JMConstants.OFFLINE_GUID;
		}
		else
		{
			guid = ident.GetId();
			SteamToGUID.Insert( ident.GetPlainId(), guid );
		}

		RemoveSyncedToClient(guid);

		inst = Players.Get( guid );
		if ( inst )
			return true;

		inst = new JMPlayerInstance( ident );

		//inst.CopyPermissions( RootPermission );
		inst.Load();

		Players.Insert( guid, inst );

		// PMPrint();

		return true;
	}

	bool OnClientDisconnected( string guid, out JMPlayerInstance inst )
	{
		Assert_Null( Players );

		inst = Players.Get( guid );
		if ( inst )
		{
			Players.Remove( guid );

			RemoveSyncedToClient(guid);

			return true;
		}
		else
		{
			return false;
		}
	}

	private void RemoveSyncedToClient(string guid)
	{
		foreach (JMPlayerInstance player: Players)
		{
			player.RemoveSyncedToClient(guid);
		}
	}

	void PMPrint()
	{
		if ( IsMissionClient() && false )
		{
			// Print( "Printing all authenticated players!" );

			for ( int i = 0; i < Players.Count(); i++ )
			{
				Players.GetElement( i ).DebugPrint();
			}
		}
	}

	string GetGUIDForSteam( string uid )
	{
		return SteamToGUID.Get( uid );
	}

	JMPlayerInstance GetPlayer( string guid )
	{
		Assert_Null( Players );

		return Players.Get( guid );
	}

	JMPlayerInstance UpdatePlayer( string guid, ParamsReadContext ctx, PlayerBase playerUpdatePlayer = NULL )
	{
		#ifdef JM_COT_DIAG_LOGGING
		auto trace = CF_Trace_2(this, "UpdatePlayer").Add(guid).Add(playerUpdatePlayer.ToString());
		#endif

		JMPlayerInstance instance = GetPlayer( guid );

		if ( !instance )
		{
			if ( !IsMissionClient() )
				return NULL;

			instance = new JMPlayerInstance( NULL, guid );
			Players.Insert( guid, instance );
		}

		instance.OnRecieve( ctx );

		if ( IsMissionClient() )
		{
			if ( m_ClientGUID == guid )
				GetModuleManager().OnClientPermissionsUpdated();

			instance.PlayerObject = playerUpdatePlayer;
		}

		return instance;
	}

	protected bool IsValidFolderForRoles( string name, FileAttr attributes )
	{
		string extenstion = ".txt";
		int strLength = name.Length();

		if ( name == extenstion )
			return false;

		if ( attributes & FileAttr.DIRECTORY )
			return false;

		if ( name == "" )
			return false;

		return true;
	}
	
	JMRole CreateRole( string name, array< string > data )
	{
		Assert_Null( Roles );

		JMRole role = GetRole( name );

		if ( !role )
		{
			role = new JMRole( name );
			Roles.Insert( name, role );
		}

		role.SerializedData.Copy( data );
		role.Deserialize();

		role.Save();

		return role;
	}

	bool LoadRole( string name, out JMRole role )
	{
		Assert_Null( Roles );

		role = GetRole( name );

		if ( !role )
		{
			role = new JMRole( name );
			Roles.Insert( name, role );

			return false;
		}

		return true;
	}

	void LoadRoleFromFile( string name )
	{
		Assert_Null( Roles );

		JMRole role = new JMRole( name );
		
		if ( role.Load() )
			Roles.Insert( name, role );
	}

	void LoadRoles()
	{
		string sName = "";
		FileAttr oFileAttr = FileAttr.INVALID;
		FindFileHandle oFileHandle = FindFile( JMConstants.DIR_ROLES + "*" + JMConstants.EXT_ROLE, sName, oFileAttr, FindFileFlags.ALL );

		if ( sName != "" )
		{
			if ( IsValidFolderForRoles( sName, oFileAttr ) )
				LoadRoleFromFile( sName.Substring( 0, sName.Length() - 4 ) );

			while ( FindNextFile( oFileHandle, sName, oFileAttr) )
			{
				if ( IsValidFolderForRoles( sName, oFileAttr ) )
					LoadRoleFromFile( sName.Substring( 0, sName.Length() - 4 ) );
			}
		}
	}

	bool RoleExists( string role )
	{
		return Roles.Contains( role );
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
	}

	JMRole GetRole( string name )
	{
		Assert_Null( Roles );

		return Roles.Get( name );
	}

	void SetMissionLoaded()
	{
		m_MissionLoaded = true;
	}
}


ref JMPermissionManager g_cot_PermissionsManager;

JMPermissionManager GetPermissionsManager()
{
	if ( !g_cot_PermissionsManager )
		g_cot_PermissionsManager = new JMPermissionManager();

	Assert_Null( g_cot_PermissionsManager );

	return g_cot_PermissionsManager;
}
#endif