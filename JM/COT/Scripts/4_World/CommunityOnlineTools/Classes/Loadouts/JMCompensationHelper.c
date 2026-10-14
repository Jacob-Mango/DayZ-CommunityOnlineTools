class JMCompensationHelper
{
	static void CreateCompensationBackup( PlayerBase player )
	{
		if ( !player || !g_Game.IsServer() )
			return;

		PlayerIdentity identity = player.GetIdentity();
		if ( !identity )
			return;

		string steamID = identity.GetPlainId();
		if ( steamID == "" )
			return;

		CreateCompensationBackup( player, steamID );
	}

	static void CreateCompensationBackup( EntityAI entity, string steamID )
	{
		if ( !entity || !g_Game.IsServer() )
			return;

	#ifdef DZ_Expansion_Core
		ExpansionPrefab exploadout = CreateExpansionLoadout( entity );
		if ( exploadout )
		{
			string exptimestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
			SaveExpansionCompensation( exploadout, steamID, exptimestamp );
			return;
		}
	#endif

		JMLoadoutItem loadoutItem = new JMLoadoutItem( entity );

		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		loadouts.Insert( loadoutItem );

		JMLoadout loadout = new JMLoadout;
		loadout.m_Items = loadouts;
		loadout.m_IsLocalPosition = false;

		string timestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
		JMLoadoutSettings.SaveCompensation( loadout, steamID, timestamp );
	}

	[Obsolete("Use CreateCompensationBackup( EntityAI entity, string ownerSteamID )")]
	static void CreateVehicleCompensationBackup( EntityAI vehicle, string ownerSteamID )
	{
		CreateCompensationBackup( vehicle, ownerSteamID );
	}

#ifdef DZ_Expansion_Core
	static ExpansionPrefab CreateExpansionLoadout( EntityAI entity )
	{
		if ( !entity )
			return null;

		ExpansionPrefab expPrefab = new ExpansionPrefab();

		if ( entity.IsMan() )
		{
			AddChildrenToExpLoadoutRecursive( expPrefab, entity );
		}
		else
		{
			expPrefab.ClassName = entity.GetType();
			AddToExpLoadoutRecursive( expPrefab, entity );
		}

		return expPrefab;
	}

	static void SaveExpansionCompensation( ExpansionPrefab prefab, string steamID, string timestamp )
	{
		string loadoutJSON;
		string errorMsg;

		if ( !JsonFileLoader<ExpansionPrefab>.MakeData( prefab, loadoutJSON, errorMsg ) )
			return;

		if ( !FileExist( JMConstants.DIR_COMPENSATIONS ) )
			MakeDirectory( JMConstants.DIR_COMPENSATIONS );

		string playerDir = JMConstants.DIR_COMPENSATIONS + steamID + "\\";
		if ( !FileExist( playerDir ) )
			MakeDirectory( playerDir );

		// Keep only the 3 most recent backups
		array< string > existingFiles = new array< string >;
		string fileName;
		FileAttr fileAttr;
		FindFileHandle findFileHandle = CF.FindFileEx( playerDir + "*.json", fileName, fileAttr, 0 );
		if ( findFileHandle )
		{
			while ( fileName != "" )
			{
				existingFiles.Insert( fileName );
				fileName = "";
				FindNextFile( findFileHandle, fileName, fileAttr );
			}
			CloseFindFile( findFileHandle );
		}

		existingFiles.Sort();

		while ( existingFiles.Count() >= 3 )
		{
			string oldestFile = existingFiles.Get( 0 );
			DeleteFile( playerDir + oldestFile );
			existingFiles.Remove( 0 );
		}

		string filepath = playerDir + timestamp + ".json";
		FileHandle file = OpenFile( filepath, FileMode.WRITE );
		if ( file )
		{
			FPrintln( file, loadoutJSON );
			CloseFile( file );
		}
	}

	static void AddChildrenToExpLoadoutRecursive( ExpansionPrefabObject loadout, EntityAI entity )
	{
		GameInventory inventory = entity.GetInventory();
		if ( !inventory )
			return;

		int i;
		EntityAI item;
		InventoryLocation il = new InventoryLocation();

		for ( i = 0; i < inventory.AttachmentCount(); ++i )
		{
			item = inventory.GetAttachmentFromIndex( i );
			item.GetInventory().GetCurrentInventoryLocation( il );
			string slotName = InventorySlots.GetSlotName( il.GetSlot() );
			loadout = loadout.BeginAttachment( item.GetType(), slotName );
			AddToExpLoadoutRecursive( loadout, item );
			loadout = loadout.End();
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( i = 0; i < cargo.GetItemCount(); ++i )
			{
				item = cargo.GetItem( i );
				loadout = loadout.BeginCargo( item.GetType() );
				AddToExpLoadoutRecursive( loadout, item );
				loadout = loadout.End();
			}
		}
	}

	static void AddToExpLoadoutRecursive( ExpansionPrefabObject loadout, EntityAI item )
	{
		loadout.Chance = 1.0;

		if ( item.HasQuantity() )
		{
			float quantity01 = item.GetQuantityNormalized();
			loadout.SetQuantity(quantity01, quantity01);
		}

		AddChildrenToExpLoadoutRecursive( loadout, item );
	}
#endif

	[Obsolete("Use new JMLoadoutItem(player) instead")]
	protected static JMLoadoutItem ProcessPlayerLoadout( PlayerBase player )
	{
		if ( !player )
			return null;

		JMLoadoutItem item = new JMLoadoutItem(player);

		return item;
	}

	[Obsolete("Use new JMLoadoutItem(vehicle) instead")]
	protected static JMLoadoutItem ProcessVehicleLoadout( EntityAI vehicle )
	{
		if ( !vehicle )
			return null;

		JMLoadoutItem item = new JMLoadoutItem(vehicle);

		return item;
	}

	[Obsolete("Use new JMLoadoutSubItem(parent) instead")]
	protected static JMLoadoutSubItem ProcessSubItem( EntityAI parent )
	{
		if ( !parent )
			return null;

		JMLoadoutSubItem item = new JMLoadoutSubItem(parent);

		return item;
	}
}
