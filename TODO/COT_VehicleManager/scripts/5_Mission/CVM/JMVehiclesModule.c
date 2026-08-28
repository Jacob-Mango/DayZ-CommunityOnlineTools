/*
	COT_VehicleManager - logique serveur

	1) Nettoyage des vehicules sur spawn (preview + delete) -- inchange.
	2) Actions par vehicule (depuis le panneau de details du menu Vehicules de COT) :
	     - RETOURNER  : remet a l'endroit un vehicule retourne (PlaceOnSurfaceAtPosition).
	     - REPARER    : repare le vehicule + les pieces deja attachees, SANS rien
	                    ajouter ni retirer (le loot en cargo n'est pas touche).
	     - RESTAURER  : repare + remet toutes les pieces par defaut (porte, capot,
	                    pneu, bougie, batterie, radiateur...) + remplit tous les fluides.

	Toutes les actions reutilisent les primitives eprouvees de COT
	(CommunityOnlineToolsBase / CarScript::COT_OnDebugSpawn) pour eviter les crashs.
*/

// ----- Reglages (modifie puis repack) ---------------------------------------

// Rayon de detection en metres (distance horizontale X/Z, l'altitude est ignoree).
const float CVM_RADIUS = 15.0;

// true = ignorer les vehicules occupes (un joueur dans un siege).
const bool CVM_SKIP_OCCUPIED = true;

// ----- Internes -------------------------------------------------------------
const float CVM_NOATTR       = -999999.0; // attribut XML absent
const int   CVM_MODE_PREVIEW = 0;
const int   CVM_MODE_DELETE  = 1;

// Actions par vehicule
const int   CVM_ACT_FLIP    = 1; // Retourner
const int   CVM_ACT_REPAIR  = 2; // Reparer
const int   CVM_ACT_RESTORE = 3; // Restaurer
// ----------------------------------------------------------------------------

modded class JMVehiclesModule
{
	void JMVehiclesModule()
	{
		// Permissions dediees (apparaissent dans la gestion des roles de COT).
		// Un superadmin (wildcard) les possede automatiquement.
		GetPermissionsManager().RegisterPermission( "Vehicles.Flip" );
		GetPermissionsManager().RegisterPermission( "Vehicles.Repair" );
		GetPermissionsManager().RegisterPermission( "Vehicles.Restore" );

		// Nettoyage sur spawn (existant)
		GetRPCManager().AddRPC( "CVM_RPC", "RPC_CVMCleanVehiclesOnSpawn", this, SingeplayerExecutionType.Server );
		GetRPCManager().AddRPC( "CVM_RPC", "RPC_CVMPreviewResult",        this, SingeplayerExecutionType.Client );

		// Actions par vehicule (nouveau)
		GetRPCManager().AddRPC( "CVM_RPC", "RPC_CVMVehicleAction",        this, SingeplayerExecutionType.Server );
	}

	// ================= SERVEUR : actions par vehicule =================
	void RPC_CVMVehicleAction( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !GetGame().IsServer() )
			return;

		Param3<int, int, int> data = new Param3<int, int, int>( 0, 0, 0 );
		if ( !ctx.Read( data ) )
		{
			Error( "[CVM] Lecture RPC action echouee" );
			return;
		}

		int netLow  = data.param1;
		int netHigh = data.param2;
		int action  = data.param3;

		string perm;
		switch ( action )
		{
		case CVM_ACT_FLIP:    perm = "Vehicles.Flip";    break;
		case CVM_ACT_REPAIR:  perm = "Vehicles.Repair";  break;
		case CVM_ACT_RESTORE: perm = "Vehicles.Restore"; break;
		default:
			return;
		}

		if ( !GetPermissionsManager().HasPermission( perm, sender ) )
			return;

		Object obj = GetGame().GetObjectByNetworkId( netLow, netHigh );
		EntityAI veh;
		if ( !Class.CastTo( veh, obj ) )
		{
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_VEH_NOTFOUND" ) );
			return;
		}

		PlayerBase player;
		Class.CastTo( player, sender.GetPlayer() );

		switch ( action )
		{
		case CVM_ACT_FLIP:
			CommunityOnlineToolsBase.PlaceOnSurfaceAtPosition( veh, veh.GetPosition() );
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_FLIP_DONE" ) );
			Print( "[CVM] FLIP vehicule " + veh.GetType() + " par " + sender.GetId() );
			break;

		case CVM_ACT_REPAIR:
			// true  = inclut les pieces attachees (repare/remplace pneu creve, etc.)
			// false = ne touche PAS au loot en cargo (rien ajoute, rien retire)
			CommunityOnlineToolsBase.HealEntityRecursive( veh, true, false );
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_REPAIR_DONE" ) );
			Print( "[CVM] REPAIR vehicule " + veh.GetType() + " par " + sender.GetId() );
			break;

		case CVM_ACT_RESTORE:
			CVM_RestoreVehicle( veh, player );
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_RESTORE_DONE" ) );
			Print( "[CVM] RESTORE vehicule " + veh.GetType() + " par " + sender.GetId() );
			break;
		}
	}

	// Restaure : pose les pieces par defaut dans les slots vides + repare + fluides.
	// Le coffre n'est pas modifie : on retire uniquement ce que le spawn vanilla y
	// ajoute, et on conserve le loot d'origine ainsi que les pieces attachees.
	private void CVM_RestoreVehicle( EntityAI veh, PlayerBase player )
	{
		// 1) Photographier le contenu actuel du coffre (references des items).
		array<EntityAI> cargoBefore = new array<EntityAI>;
		CargoBase cargo = veh.GetInventory().GetCargo();
		if ( cargo )
		{
			for ( int i = 0; i < cargo.GetItemCount(); i++ )
			{
				EntityAI it = cargo.GetItem( i );
				if ( it )
					cargoBefore.Insert( it );
			}
		}

		// 2) Spawn vanilla (securise par COT) : pose les BONNES pieces par defaut
		//    (porte, capot, pneu, bougie, batterie, radiateur...) dans les slots
		//    vides. Effet de bord : peut deposer des items par defaut dans le coffre.
		CarScript car;
		BoatScript boat;
		if ( Class.CastTo( car, veh ) )
			car.COT_OnDebugSpawn( player );
		else if ( Class.CastTo( boat, veh ) )
			boat.COT_OnDebugSpawn( player );

		// 3) Nettoyer le coffre : supprimer UNIQUEMENT les items ajoutes par le spawn
		//    (ceux absents de la photo). Le loot d'origine est conserve, les pieces
		//    attachees (qui ne sont pas dans le coffre) ne sont pas touchees.
		cargo = veh.GetInventory().GetCargo();
		if ( cargo )
		{
			array<EntityAI> toRemove = new array<EntityAI>;
			for ( int j = 0; j < cargo.GetItemCount(); j++ )
			{
				EntityAI item = cargo.GetItem( j );
				if ( item && cargoBefore.Find( item ) == -1 )
					toRemove.Insert( item );
			}

			foreach ( EntityAI dead : toRemove )
				GetGame().ObjectDelete( dead );
		}

		// 4) Reparer le vehicule + les pieces (sans toucher au coffre).
		CommunityOnlineToolsBase.HealEntityRecursive( veh, true, false );

		// 5) Tous les fluides au maximum (carburant, huile, frein, liquide de refroid.).
		CommunityOnlineToolsBase.Refuel( veh );
	}

	// ================= SERVEUR : nettoyage sur spawn (existant) =================
	void RPC_CVMCleanVehiclesOnSpawn( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		if ( !GetGame().IsServer() )
			return;

		if ( !GetPermissionsManager().HasPermission( "Vehicles.Delete.All", sender ) )
			return;

		Param2<float, int> data = new Param2<float, int>( 0, 0 );
		if ( !ctx.Read( data ) )
		{
			Error( "[CVM] Lecture RPC echouee" );
			return;
		}

		float radius = data.param1;
		int   mode   = data.param2;
		if ( radius <= 0 )
			radius = CVM_RADIUS;

		array<vector> spawns = new array<vector>;
		if ( CVM_LoadVehicleSpawns( spawns ) == 0 )
		{
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_NoSpawns" ) );
			return;
		}

		bool doDelete = ( mode == CVM_MODE_DELETE );
		int scanned = 0;
		int matched = 0;
		int deleted = 0;
		array<vector> matchedPositions = new array<vector>;
		CVM_ScanFleet( spawns, radius, doDelete, scanned, matched, deleted, matchedPositions );

		if ( doDelete )
		{
			COTCreateNotification( sender, new StringLocaliser( "#STR_CVM_Deleted", deleted.ToString(), scanned.ToString() ) );
			Print( "[CVM] DELETE " + deleted + "/" + scanned + " (rayon " + (int)radius + "m) par " + sender.GetId() );
		}
		else
		{
			// Renvoie compte + positions au client (colorisation + confirmation).
			GetRPCManager().SendRPC( "CVM_RPC", "RPC_CVMPreviewResult", new Param3<int, int, ref array<vector>>( scanned, (int)radius, matchedPositions ), true, sender );
			Print( "[CVM] PREVIEW " + matched + "/" + scanned + " (rayon " + (int)radius + "m) pour " + sender.GetId() );
		}
	}

	private void CVM_ScanFleet( array<vector> spawns, float radius, bool doDelete, out int scanned, out int matched, out int deleted, array<vector> matchedOut )
	{
		// Voitures
		auto carNode = CarScript.s_JM_AllCars.m_Head;
		while ( carNode )
		{
			CarScript car = carNode.m_Value;
			carNode = carNode.m_Next; // avancer AVANT une eventuelle suppression
			CVM_ProcessVehicle( car, spawns, radius, doDelete, scanned, matched, deleted, matchedOut );
		}

		// Bateaux
		auto boatNode = BoatScript.s_JM_AllBoats.m_Head;
		while ( boatNode )
		{
			BoatScript boat = boatNode.m_Value;
			boatNode = boatNode.m_Next;
			CVM_ProcessVehicle( boat, spawns, radius, doDelete, scanned, matched, deleted, matchedOut );
		}
	}

	private void CVM_ProcessVehicle( EntityAI veh, array<vector> spawns, float radius, bool doDelete, out int scanned, out int matched, out int deleted, array<vector> matchedOut )
	{
		if ( !veh )
			return;
		if ( veh.IsSetForDeletion() )
			return;

		scanned++;

		if ( CVM_SKIP_OCCUPIED && CVM_HasCrew( veh ) )
			return;

		if ( !CVM_NearAnySpawn( veh.GetPosition(), spawns, radius ) )
			return;

		matched++;

		if ( matchedOut )
			matchedOut.Insert( veh.GetPosition() );

		if ( doDelete )
		{
			veh.DeleteSafe();
			deleted++;
		}
	}

	private bool CVM_HasCrew( EntityAI veh )
	{
		Transport trans = Transport.Cast( veh );
		if ( !trans )
			return false;

		int seats = trans.CrewSize();
		for ( int i = 0; i < seats; i++ )
		{
			if ( trans.CrewMember( i ) )
				return true;
		}
		return false;
	}

	private bool CVM_NearAnySpawn( vector pos, array<vector> spawns, float radius )
	{
		float r2 = radius * radius;
		foreach ( vector s : spawns )
		{
			float dx = pos[0] - s[0];
			float dz = pos[2] - s[2];
			if ( ( dx * dx + dz * dz ) <= r2 )
				return true;
		}
		return false;
	}

	// ================= CLIENT =================
	void RPC_CVMPreviewResult( CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target )
	{
		Param3<int, int, ref array<vector>> data = new Param3<int, int, ref array<vector>>( 0, 0, new array<vector> );
		if ( !ctx.Read( data ) )
			return;

		int scanned = data.param1;
		int radius  = data.param2;
		array<vector> matchedPositions = data.param3;
		if ( !matchedPositions )
			matchedPositions = new array<vector>;

		JMVehiclesMenu menu;
		if ( Class.CastTo( menu, GetForm() ) )
			menu.CVM_ApplyPreview( scanned, radius, matchedPositions );
		else
			COTCreateLocalAdminNotification( new StringLocaliser( "#STR_CVM_Count", matchedPositions.Count().ToString(), scanned.ToString(), radius.ToString() ) );
	}

	// ================= Lecture cfgeventspawns.xml =================
	private int CVM_LoadVehicleSpawns( out array<vector> outSpawns )
	{
		outSpawns.Clear();

		string path = "$mission:cfgeventspawns.xml";
		FileHandle file = OpenFile( path, FileMode.READ );
		if ( !file )
		{
			Print( "[CVM] Impossible d'ouvrir " + path );
			return 0;
		}

		bool inVehicleEvent = false;
		string line;

		while ( true )
		{
			int charCount = FGets( file, line );
			if ( charCount < 0 )
				break; // fin de fichier

			if ( line.Contains( "<event" ) )
			{
				inVehicleEvent = CVM_IsVehicleEvent( CVM_ReadAttr( line, "name" ) );
				continue;
			}

			if ( line.Contains( "</eventposdef>" ) )
				break;

			if ( inVehicleEvent && line.Contains( "<pos" ) )
			{
				float x = CVM_ReadAttrF( line, "x" );
				float z = CVM_ReadAttrF( line, "z" );
				if ( x != CVM_NOATTR && z != CVM_NOATTR )
					outSpawns.Insert( Vector( x, 0, z ) );
			}
		}

		CloseFile( file );
		Print( "[CVM] " + outSpawns.Count() + " positions de spawn vehicule lues dans cfgeventspawns.xml" );
		return outSpawns.Count();
	}

	// Event "vehicule" : nom commence par "Vehicle" ou contient "Boat".
	// Les events "Static..." (ex : StaticPoliceCar) sont volontairement exclus.
	private bool CVM_IsVehicleEvent( string evName )
	{
		if ( evName == "" )
			return false;
		if ( evName.IndexOf( "Vehicle" ) == 0 )
			return true;
		if ( evName.Contains( "Boat" ) )
			return true;
		return false;
	}

	private string CVM_ReadAttr( string line, string attr )
	{
		string token = attr + "=\"";
		int start = line.IndexOf( token );
		if ( start < 0 )
			return "";

		start += token.Length();
		string rest = line.Substring( start, line.Length() - start );

		int end = rest.IndexOf( "\"" );
		if ( end < 0 )
			return "";

		return rest.Substring( 0, end );
	}

	private float CVM_ReadAttrF( string line, string attr )
	{
		string v = CVM_ReadAttr( line, attr );
		if ( v == "" )
			return CVM_NOATTR;
		return v.ToFloat();
	}
}
