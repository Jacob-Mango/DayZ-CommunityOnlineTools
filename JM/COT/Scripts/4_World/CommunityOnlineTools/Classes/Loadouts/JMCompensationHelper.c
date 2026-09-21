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

	#ifdef DZ_Expansion_Core
		ExpansionPrefab exploadout = CreateExpansionLoadout( player );
		if ( exploadout )
		{
			string exptimestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
			SaveExpansionCompensation( exploadout, steamID, exptimestamp );
			return;
		}
	#endif

		JMLoadoutItem loadoutItem = ProcessPlayerLoadout( player );
		if ( !loadoutItem )
			return;

		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		loadouts.Insert( loadoutItem );

		JMLoadout loadout = new JMLoadout;
		loadout.m_Items = loadouts;
		loadout.m_IsLocalPosition = false;

		string timestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
		JMLoadoutSettings.SaveCompensation( loadout, steamID, timestamp );
	}

	static void CreateVehicleCompensationBackup( EntityAI vehicle, string ownerSteamID )
	{
		if ( !vehicle || !g_Game.IsServer() )
			return;

		if ( ownerSteamID == "" )
			return;

	#ifdef DZ_Expansion_Core
		ExpansionPrefab exploadout = CreateExpansionLoadout( vehicle );
		if ( exploadout )
		{
			string exptimestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
			SaveExpansionCompensation( exploadout, ownerSteamID, exptimestamp );
			return;
		}
	#endif

		JMLoadoutItem loadoutItem = ProcessVehicleLoadout( vehicle );
		if ( !loadoutItem )
			return;

		array< ref JMLoadoutItem > loadouts = new array< ref JMLoadoutItem >;
		loadouts.Insert( loadoutItem );

		JMLoadout loadout = new JMLoadout;
		loadout.m_Items = loadouts;
		loadout.m_IsLocalPosition = false;

		string timestamp = JMDate.Now().ToString( "YYYY-MM-DD_hh-mm-ss" );
		JMLoadoutSettings.SaveCompensation( loadout, ownerSteamID, timestamp );
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
		FindFileHandle findFileHandle = FindFile( playerDir + "*.json", fileName, fileAttr, 0 );
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

	static void AddChildrenToExpLoadoutRecursive( ExpansionPrefab prefab, EntityAI entity )
	{
		if ( !prefab || !entity )
			return;

		GameInventory inventory = entity.GetInventory();
		if ( !inventory )
			return;

		int i;
		EntityAI item;
		InventoryLocation il = new InventoryLocation();

		for ( i = 0; i < inventory.AttachmentCount(); ++i )
		{
			item = inventory.GetAttachmentFromIndex( i );
			if ( !item )
				continue;

			if ( item.GetInventory() )
				item.GetInventory().GetCurrentInventoryLocation( il );

			string slotName = InventorySlots.GetSlotName( il.GetSlot() );
			ExpansionPrefab childPrefab = ExpansionPrefab.Cast( prefab.BeginAttachment( item.GetType(), slotName ) );
			if ( childPrefab )
			{
				AddToExpLoadoutRecursive( childPrefab, item );
				childPrefab.End();
			}
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( i = 0; i < cargo.GetItemCount(); ++i )
			{
				item = cargo.GetItem( i );
				if ( !item )
					continue;

				ExpansionPrefab cargoPrefab = ExpansionPrefab.Cast( prefab.BeginCargo( item.GetType() ) );
				if ( cargoPrefab )
				{
					AddToExpLoadoutRecursive( cargoPrefab, item );
					cargoPrefab.End();
				}
			}
		}
	}

	static void AddToExpLoadoutRecursive( ExpansionPrefab prefab, EntityAI item )
	{
		if ( !prefab || !item )
			return;

		prefab.Chance = 1.0;

		if ( item.HasQuantity() )
		{
			float quantity01 = item.GetQuantityNormalized();
			prefab.SetQuantity( quantity01, quantity01 );
		}

		AddChildrenToExpLoadoutRecursive( prefab, item );
	}
#endif

	protected static JMLoadoutItem ProcessPlayerLoadout( PlayerBase player )
	{
		if ( !player )
			return null;

		JMLoadoutItem item = new JMLoadoutItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname = player.GetType();
		item.m_LocalPosition = player.GetPosition();
		item.m_LocalRotation = player.GetOrientation();

		item.m_Data = dataItem;
		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		if ( !player.IsInherited( Building ) && !player.IsInherited( AdvancedCommunication ) )
			dataItem.m_Health = player.GetHealth();

		if ( player.HasQuantity() )
		{
			dataItem.m_Quantity = player.GetQuantity();
			dataItem.m_LiquidType = player.GetLiquidType();
		}

		dataItem.m_Temperature = player.GetTemperature();

		GameInventory inventory = player.GetInventory();
		if ( !inventory )
			return item;

		ItemBase child;
		for ( int k = 0; k < inventory.AttachmentCount(); k++ )
		{
			child = ItemBase.Cast( inventory.GetAttachmentFromIndex( k ) );
			if ( child )
				item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = ItemBase.Cast( cargo.GetItem( j ) );
				if ( child )
					item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}

	protected static JMLoadoutItem ProcessVehicleLoadout( EntityAI vehicle )
	{
		if ( !vehicle )
			return null;

		JMLoadoutItem item = new JMLoadoutItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname = vehicle.GetType();
		item.m_LocalPosition = vehicle.GetPosition();
		item.m_LocalRotation = vehicle.GetOrientation();

		item.m_Data = dataItem;
		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		dataItem.m_Health = vehicle.GetHealth();

		GameInventory inventory = vehicle.GetInventory();
		if ( !inventory )
			return item;

		EntityAI child;
		for ( int k = 0; k < inventory.AttachmentCount(); k++ )
		{
			child = EntityAI.Cast( inventory.GetAttachmentFromIndex( k ) );
			if ( child )
				item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = EntityAI.Cast( cargo.GetItem( j ) );
				if ( child )
					item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}

	protected static JMLoadoutSubItem ProcessSubItem( EntityAI parent )
	{
		if ( !parent )
			return null;

		JMLoadoutSubItem item = new JMLoadoutSubItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname = parent.GetType();

		ItemBase itembs;
		if ( Class.CastTo( itembs, parent ) )
		{
			if ( !itembs.IsInherited( Building ) && !itembs.IsInherited( AdvancedCommunication ) )
				dataItem.m_Health = itembs.GetHealth();

			if ( itembs.HasQuantity() )
			{
				dataItem.m_Quantity = itembs.GetQuantity();
				dataItem.m_LiquidType = itembs.GetLiquidType();
			}

			dataItem.m_Temperature = itembs.GetTemperature();
		}

		item.m_Data = dataItem;

		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		GameInventory inventory = parent.GetInventory();
		if ( !inventory )
			return item;

		EntityAI child;
		for ( int k = 0; k < inventory.AttachmentCount(); k++ )
		{
			child = EntityAI.Cast( inventory.GetAttachmentFromIndex( k ) );
			if ( child )
				item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = EntityAI.Cast( cargo.GetItem( j ) );
				if ( child )
					item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}
}
