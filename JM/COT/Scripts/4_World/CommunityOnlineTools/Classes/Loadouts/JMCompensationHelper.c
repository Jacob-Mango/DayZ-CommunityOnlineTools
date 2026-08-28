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
		GameInventory inventory = entity.GetInventory();
		int i;
		EntityAI item;
		InventoryLocation il = new InventoryLocation();

		for ( i = 0; i < inventory.AttachmentCount(); ++i )
		{
			item = inventory.GetAttachmentFromIndex( i );
			item.GetInventory().GetCurrentInventoryLocation( il );
			string slotName = InventorySlots.GetSlotName( il.GetSlot() );
			prefab = ExpansionPrefab.Cast(prefab.BeginAttachment( item.GetType(), slotName ));
			AddToExpLoadoutRecursive( prefab, item );
			prefab = ExpansionPrefab.Cast(prefab.End());
		}

		CargoBase cargo = inventory.GetCargo();
		if ( cargo )
		{
			for ( i = 0; i < cargo.GetItemCount(); ++i )
			{
				item = cargo.GetItem( i );
				prefab = ExpansionPrefab.Cast(prefab.BeginCargo( item.GetType() ));
				AddToExpLoadoutRecursive( prefab, item );
				prefab = ExpansionPrefab.Cast( prefab.End()) ;
			}
		}
	}

	static void AddToExpLoadoutRecursive( ExpansionPrefab prefab, EntityAI item )
	{
		prefab.Chance = 1.0;

		if ( item.HasQuantity() )
		{
			float quantity01 = item.GetQuantityNormalized();
			prefab.SetQuantity( quantity01, quantity01 );
		}

		AddChildrenToExpLoadoutRecursive( prefab, item );
	}
#endif

	private static JMLoadoutItem ProcessPlayerLoadout( PlayerBase player )
	{
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

		ItemBase child;
		for ( int k = 0; k < player.GetInventory().AttachmentCount(); k++ )
		{
			child = ItemBase.Cast( player.GetInventory().GetAttachmentFromIndex( k ) );
			item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = player.GetInventory().GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = ItemBase.Cast( cargo.GetItem( j ) );
				item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}

	private static JMLoadoutItem ProcessVehicleLoadout( EntityAI vehicle )
	{
		JMLoadoutItem item = new JMLoadoutItem;
		JMLoadoutItemData dataItem = new JMLoadoutItemData;

		item.m_Classname = vehicle.GetType();
		item.m_LocalPosition = vehicle.GetPosition();
		item.m_LocalRotation = vehicle.GetOrientation();

		item.m_Data = dataItem;
		item.m_Attachments = new array< ref JMLoadoutSubItem >;

		dataItem.m_Health = vehicle.GetHealth();

		EntityAI child;
		for ( int k = 0; k < vehicle.GetInventory().AttachmentCount(); k++ )
		{
			child = EntityAI.Cast( vehicle.GetInventory().GetAttachmentFromIndex( k ) );
			item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = vehicle.GetInventory().GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = EntityAI.Cast( cargo.GetItem( j ) );
				item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}

	private static JMLoadoutSubItem ProcessSubItem( EntityAI parent )
	{
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

		EntityAI child;
		for ( int k = 0; k < parent.GetInventory().AttachmentCount(); k++ )
		{
			child = EntityAI.Cast( parent.GetInventory().GetAttachmentFromIndex( k ) );
			item.m_Attachments.Insert( ProcessSubItem( child ) );
		}

		CargoBase cargo = parent.GetInventory().GetCargo();
		if ( cargo )
		{
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				child = EntityAI.Cast( cargo.GetItem( j ) );
				item.m_Attachments.Insert( ProcessSubItem( child ) );
			}
		}

		return item;
	}
}
