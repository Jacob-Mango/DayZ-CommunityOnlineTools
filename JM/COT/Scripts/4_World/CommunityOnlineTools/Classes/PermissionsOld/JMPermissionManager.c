#ifndef CF_MODULE_PERMISSIONS
class JMPermissionManager
{
	ref map< string, ref JMPlayerInstance > Players;
	ref map< string, ref JMRole > Roles;
	ref map< string, string > SteamToGUID;
	ref JMPermission RootPermission;
	protected string m_ClientGUID;

	//! protected, not private: DayZ-Expansion's `modded class JMPermissionManager`
	//! reads this in its Expansion_RegisterPermission override. A modded class
	//! cannot touch a private member of the class it mods, so privatising this
	//! breaks Expansion's compile. Keep protected for mod compatibility.
	protected bool m_MissionLoaded;

	//! DIAG test roster only - see CreateFakePlayers.
	protected bool m_FakePlayersCreated;
	protected ref TStringArray m_FakeNamesUsed;
	protected static int s_FakePlayerSeq;

	//! Bumped whenever Players gains/loses an entry or a role assignment
	//! changes, so UI consumers can skip a rebuild when the roster hasn't
	//! actually changed since their last poll.
	protected int m_RosterVersion;

	void JMPermissionManager()
	{
		Players = new map< string, ref JMPlayerInstance >;
		Roles = new map< string, ref JMRole >;

		SteamToGUID = new map< string, string >;

		RootPermission = new JMPermission( JMConstants.PERM_ROOT );
		
		if ( IsMissionHost() )
		{
			MakeDirectory( JMConstants.DIR_PF );

		#ifdef JM_COT_ENABLE_INDIVIDUAL_PERMS
			MakeDirectory( JMConstants.DIR_PERMISSIONS );
		#endif

			MakeDirectory( JMConstants.DIR_PLAYERS );
			MakeDirectory( JMConstants.DIR_ROLES );
		}
		
		Assert_Null( Players );
		Assert_Null( Roles );
		Assert_Null( SteamToGUID );
		Assert_Null( RootPermission );

	#ifdef DIAG_DEVELOPER
		if (FileExist("$profile:COT_RegisteredPermissions.txt"))
			DeleteFile("$profile:COT_RegisteredPermissions.txt");
	#endif
	}

	string GetClientGUID()
	{
		return m_ClientGUID;
	}

	JMPlayerInstance GetClientPlayer()
	{
		return Players.Get( m_ClientGUID );
	}

	string GetGUIDForSteam( string uid )
	{
		return SteamToGUID.Get( uid );
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

	protected void GetPermissionsAsList( JMPermission permission, string indent, inout array< JMPermission > permissions, bool last = false )
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

	JMPlayerInstance GetPlayer( string guid )
	{
		Assert_Null( Players );

		return Players.Get( guid );
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

	JMRole GetRole( string name )
	{
		Assert_Null( Roles );

		return Roles.Get( name );
	}

	void GetRolesAsList( out array< JMRole > roles, bool sorted = false )
	{
		Assert_Null( Roles );

		if ( roles == NULL )
			roles = new array< JMRole >();

		if (sorted)
		{
			TStringArray names = Roles.GetKeyArray();
			names.Sort();
			foreach (string name: names)
				roles.Insert(Roles[name]);
		}
		else
		{
			roles.Copy( Roles.GetValueArray() );
		}

	#ifdef COT_ROLES_DEBUG
		roles.Debug();
	#endif
	}

	JMPermission GetRootPermission()
	{
		Assert_Null( RootPermission );

		return RootPermission;
	}

	int GetRosterVersion()
	{
		return m_RosterVersion;
	}

	bool HasPermission( string permission, out JMPlayerInstance instance = null )
	{
		if ( IsMissionClient() ) 
		{
			instance = GetClientPlayer();

			if ( IsMissionHost() )
				return true;
			
			if ( !instance )
				return false;

			return instance.HasPermission( permission );
		}

		//! A dedicated server has no "own" player to check. Answer false (never
		//! grant on a wrong call) and raise Error since this is a programming mistake
		//! in the caller that needs to be fixed (wrong form of HasPermission used).
		Error("HasPermission( permission ) was called on a server - pass the identity: HasPermission( permission, identity ). Permission: " + permission );

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

			//! Own client player is not registered yet during the connect race -
			//! permission checks can run this early (sidebar Init, an incoming
			//! RPC_UpdateRole) before it is. Expected and recoverable, same as
			//! the other HasPermission() overload above - not a programming
			//! mistake, so no Assert_Null spam for it.
			if ( !instance )
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

		return allowed;
	}

	/**
	 * @brief check permission when it's related to an RPC received on server
	 *
	 * This overload is the single choke point every incoming COT RPC goes
	 * through on the server, and a refusal here means a client asked for
	 * something its own UI never offers it. That is close to the strongest
	 * signal available from inside a mod: a legitimate client cannot
	 * produce it by accident. Reported, not acted on - the anti-cheat
	 * module decides whether a burst of these is worth a flag.
	 */
	bool HasPermissionRPC( string permission, PlayerIdentity ihp )
	{
		if ( IsMissionOffline() )
			return true;

		JMPlayerInstance instance;
		return HasPermissionRPC( permission, ihp, instance );
	}

	bool HasPermissionRPC( string permission, PlayerIdentity identity, out JMPlayerInstance instance )
	{
		bool allowed = HasPermission(permission, identity, instance);

	#ifdef SERVER
		if ( !allowed )
			JMAntiCheatSignals.ReportDeniedRpc( identity.GetId(), permission );
	#endif

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

		//! Reached only when nothing decided early: with requireAll every
		//! permission was held, without it none was. Returning true here for
		//! "any of" granted access to a caller holding none of them.
		return requireAll;
	}

	bool HasQuickActionAccess(PlayerBase player)
	{
	#ifdef SERVER
		PlayerIdentity identity = player.GetIdentity();
		if (identity && GetCommunityOnlineToolsBase().IsActive(identity) && HasPermission(JMConstants.PERM_ACTIONS_QUICKACTIONS, identity))
	#else
		if (GetCommunityOnlineToolsBase().IsActive() && HasPermission(JMConstants.PERM_ACTIONS_QUICKACTIONS))
	#endif
			return true;

		return false;
	}

	bool IsRole( string role )
	{
		return Roles.Contains( role );
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

	void SetClientGUID( string guid )
	{
		m_ClientGUID = guid;
	}

	void SetMissionLoaded()
	{
		m_MissionLoaded = true;
	}

	void ResetMission()
	{
		Assert_Null( Players );
		Assert_Null( Roles );

		m_FakePlayersCreated = false;

		if ( !IsMissionClient() )
			return;

		Players.Clear();
		Roles.Clear();
	}

	//! Test roster. SERVER ONLY: the client gets these the same way it gets real
	//! players, through the permission sync. Run on both ends it built two
	//! independent sets - the client's own thirty plus the thirty the server
	//! pushed - and because both draw from the same name pools the roster came
	//! out looking like every player was listed two or three times.
	//!
	//! Also guards against being called twice on the same manager, which would
	//! reuse the same `i` values for a second batch.
	void CreateFakePlayers()
	{
		if ( !g_Game.IsServer() )
			return;

		if ( m_FakePlayersCreated )
			return;

		m_FakePlayersCreated = true;

		vector basePos = "7500 0 7500".ToVector();
		if ( g_Game.GetPlayer() )
		{
			basePos = g_Game.GetPlayer().GetPosition();
			if ( basePos == vector.Zero )
				basePos = "7500 0 7500".ToVector();
		}

		//! Drawn without replacement so no two test players share a name. A
		//! repeated name in a test roster is indistinguishable from a duplicated
		//! row, and the whole point of this roster is to spot list bugs.
		m_FakeNamesUsed = new TStringArray;

		for ( int i = 0; i < 30; i++ )
		{
			CreateFakePlayer( i, basePos );
		}

		m_FakeNamesUsed = NULL;
	}

	protected EntityAI CreateItemWithRandomHealth( EntityAI parent, string classname )
	{
		if ( !parent || classname == "" || !parent.GetInventory() )
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

	protected void CreateWeaponForSurvivor( PlayerBase survivor )
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

	protected void CreateFakePlayer( int i, vector basePos )
	{
		array<string> firstNames = { "Adam", "Alex", "Alice", "Arthur", "Ben", "Charlie", "Chloe", "Daniel", "David", "Emma", "Ethan", "Fiona", "George", "Hannah", "Ian", "Jack", "Julia", "Kevin", "Laura", "Liam", "Lucas", "Marcus", "Mia", "Noah", "Oliver", "Rachel", "Sam", "Sophie", "Victor", "Zack" };
		array<string> lastNames = { "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Wilson", "Taylor", "Anderson", "Thomas", "White", "Harris", "Martin", "Thompson", "Garcia", "Martinez", "Robinson", "Clark", "Rodriguez", "Lewis", "Lee", "Walker", "Hall", "Allen", "Young", "Hernandez", "King", "Wright", "Lopez" };

		string fakeName = "";
		int nameTry = 0;

		//! Re-roll a name that is already on the roster. Bounded rather than a
		//! plain while: the pools give 900 combinations for 30 players, so a
		//! free name is found in a couple of tries, and the fallback below still
		//! has to be there for the day somebody raises the count.
		while ( nameTry < 32 )
		{
			fakeName = firstNames.GetRandomElement() + " " + lastNames.GetRandomElement();

			if ( !m_FakeNamesUsed || m_FakeNamesUsed.Find( fakeName ) < 0 )
				break;

			nameTry++;
		}

		if ( m_FakeNamesUsed )
		{
			if ( m_FakeNamesUsed.Find( fakeName ) >= 0 )
				fakeName = fakeName + " " + ( i + 1 );

			m_FakeNamesUsed.Insert( fakeName );
		}

		//! Sequential, not random: a random suffix collides eventually, and two
		//! test players sharing a guid is the exact bug this roster exists to
		//! make visible.
		s_FakePlayerSeq++;
		string fakeGuid = "GFake_" + s_FakePlayerSeq;
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

	#ifdef DIAG_DEVELOPER
		//CF.FormatErrorEx("Registered permission %1 %2", ErrorExSeverity.INFO, permission, typename.EnumToString(JMPermissionType, permType));
		FileHandle handle = OpenFile("$profile:COT_RegisteredPermissions.txt", FileMode.APPEND);
		if (handle)
		{
			string stack;
			DumpStackString(stack);

			TStringArray lines = {};
			stack.Split("\n", lines);

			stack = "";

			int lastIndex = lines.Count() - 1;
			for (int i = lastIndex; i >= 0; --i)
			{
				if (i == lastIndex)
				{
					lines[i] = "";
				}
				else
				{
					string line = lines[i];
					int a = line.IndexOf("#");
					if (a > -1)
					{
						int b = line.IndexOf("(");
						if (b > a)
							lines[i] = line.Substring(0, a) + line.Substring(b, line.Length() - b - 1);
					}
				}
			}

			stack = string.Join("\n", lines);

			FPrintln(handle, string.Format("Registered permission %1 %2", permission, typename.EnumToString(JMPermissionType, permType)));
			FPrintln(handle, stack);
			FPrintln(handle, "");

			CloseFile(handle);
		}
	#endif
	}

	array< string > Serialize()
	{
		auto trace = CF_Trace_1(this, "Serialize").Add(RootPermission.Children.Count());

		Assert_Null( RootPermission );

		array< string > data = new array< string >;
		RootPermission.Serialize( data );
		return data;
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
		m_RosterVersion++;

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
			m_RosterVersion++;

			RemoveSyncedToClient(guid);

			return true;
		}
		else
		{
			return false;
		}
	}

	protected void RemoveSyncedToClient(string guid)
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
			m_RosterVersion++;
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

	JMRole CreateRole( string name, array< string > data = null )
	{
		JMRole role;

		LoadRole( name, role );

		if (data)
		{
			role.SerializedData.Copy( data );
			role.Deserialize();
		}

		role.Save();

		return role;
	}

	JMRole CreateRoleEx( string name, JMPermissionType type = JMPermissionType.INHERIT )
	{
		JMRole role;

		LoadRole( name, role );

		if ( type != JMPermissionType.INHERIT )
			role.ChangePermissionTypeRecursive( JMPermissionType.INHERIT, type );

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