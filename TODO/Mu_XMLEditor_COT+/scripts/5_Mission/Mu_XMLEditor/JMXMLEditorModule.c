/*
	JMXMLEditorModule
	------------------
	Modul COT (Community Online Tools) dodajacy funkcje Edytora XML znana
	z moda VPPAdminTools. Logika edycji (JMXMLParser) zostala przeniesiona
	1:1, natomiast cala "instalacja" w grze (rejestracja modulu, uprawnien,
	RPC, webhookow) korzysta wylacznie z natywnego API COT - nie z API VPP.

	WAZNE: kazde RPC pisze/czyta pojedyncze wartosci po kolei (dokladnie tak
	jak robi to natywny kod COT, np. JMPlayerModule::TeleportTo) - bez
	pakowania w obiekty Param2/Param3 po stronie nadawcy z odczytem innego
	ksztaltu po stronie odbiorcy.
*/

class JMXMLEditorModule : JMRenderableModuleBase
{
	private string XML_PATH_BACKUP  = "$profile:Mu_XMLEditor/Backups/";
	private string MISSION_XML_PATH = "$mission:db/types.xml";

	void JMXMLEditorModule()
	{
		Print( "[Mu_XMLEditor][DEBUG] JMXMLEditorModule() konstruktor wywolany. IsMissionHost=" + IsMissionHost().ToString() + " IsMissionClient=" + IsMissionClient().ToString() );

		GetPermissionsManager().RegisterPermission( "Admin.XMLEditor.View" );
		GetPermissionsManager().RegisterPermission( "Admin.XMLEditor.Edit" );
		GetPermissionsManager().RegisterPermission( "Admin.XMLEditor.ScanLoot" );

		Print( "[Mu_XMLEditor][DEBUG] Uprawnienia zarejestrowane (View/Edit/ScanLoot)." );
	}

	override bool HasAccess()
	{
		bool access = GetPermissionsManager().HasPermission( "Admin.XMLEditor.View" );
		Print( "[Mu_XMLEditor][DEBUG] HasAccess() wywolane, wynik = " + access.ToString() );
		return access;
	}

	override string GetInputToggle()
	{
		return "UACOTToggleXMLEditor";
	}

	override string GetLayoutRoot()
	{
		string path = "Mu_XMLEditor/GUI/layouts/xmleditor_form.layout";
		Print( "[Mu_XMLEditor][DEBUG] GetLayoutRoot() wywolane, zwraca: " + path );
		return path;
	}

	override string GetTitle()
	{
		return "XML Editor";
	}

	override string GetIconName()
	{
		return "X";
	}

	override bool ImageIsIcon()
	{
		return false;
	}

	override string GetWebhookTitle()
	{
		return "XML Editor Module";
	}

	override void GetWebhookTypes( out array< string > types )
	{
		types.Insert( "Edit" );
		types.Insert( "Delete" );
		types.Insert( "Scan" );
	}

	// ------------------------------------------------------------------
	// RPC routing
	// ------------------------------------------------------------------

	override int GetRPCMin()
	{
		return JMXMLEditorModuleRPC.INVALID;
	}

	override int GetRPCMax()
	{
		return JMXMLEditorModuleRPC.COUNT;
	}

	override void OnRPC( PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx )
	{
		super.OnRPC( sender, target, rpc_type, ctx );

		switch ( rpc_type )
		{
		// --- przychodzace na serwerze (zadania od klienta) ---
		case JMXMLEditorModuleRPC.GetTypesFiles:
			RPC_GetTypesFiles( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.GetTypesFromFile:
			RPC_GetTypesFromFile( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.GetDetails:
			RPC_GetDetails( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.SaveChanges:
			RPC_SaveChanges( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.ScanLoot:
			RPC_ScanLoot( ctx, sender, target );
			break;

		// --- przychodzace na kliencie (odpowiedzi serwera, trafiaja do aktywnego formularza) ---
		case JMXMLEditorModuleRPC.HandleTypesFiles:
			RPC_HandleTypesFiles( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.HandleTypesList:
			RPC_HandleTypesList( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.HandleDetails:
			RPC_HandleDetails( ctx, sender, target );
			break;
		case JMXMLEditorModuleRPC.HandleScanResults:
			RPC_HandleScanResults( ctx, sender, target );
			break;
		}
	}

	// ------------------------------------------------------------------
	// 1) Lista plikow types.xml  (dwie rownolegle tablice: sciezki + nazwy)
	// ------------------------------------------------------------------

	void RequestTypesFiles()
	{
		if ( IsMissionHost() )
		{
			Exec_GetTypesFiles( NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send( NULL, JMXMLEditorModuleRPC.GetTypesFiles, true, NULL );
		}
	}

	private void RPC_GetTypesFiles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		Exec_GetTypesFiles( senderRPC );
	}

	private void Exec_GetTypesFiles( PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.View", ident, instance ) )
			return;

		array<string> paths = new array<string>;
		array<string> names = new array<string>;

		paths.Insert( MISSION_XML_PATH );
		names.Insert( "types.xml (root)" );

		FindAdditionalXmlFiles( paths, names );

		if ( ident != NULL )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( paths );
			rpc.Write( names );
			rpc.Send( NULL, JMXMLEditorModuleRPC.HandleTypesFiles, true, ident );
		}
	}

	// Skanuje cfgeconomycore.xml linia po linii (bez zaleznosci od CF.XML) i
	// wyciaga dodatkowe pliki types zarejestrowane przez inne mody/mapy.
	private void FindAdditionalXmlFiles( inout array<string> paths, inout array<string> names )
	{
		FileHandle handle = OpenFile( "$mission:cfgeconomycore.xml", FileMode.READ );
		if ( handle == 0 )
		{
			Print( "[Mu_XMLEditor] Nie znaleziono cfgeconomycore.xml" );
			return;
		}

		string line = "";
		string currentFolder = "";

		while ( FGets( handle, line ) != -1 )
		{
			if ( line.IndexOf( "<ce folder=" ) != -1 )
			{
				currentFolder = ExtractAttribute( line, "folder" );
				continue;
			}

			if ( line.IndexOf( "type=\"types\"" ) != -1 )
			{
				string fileName = ExtractAttribute( line, "name" );
				if ( fileName != "" )
				{
					string finalPath = string.Format( "$mission:%1/%2", currentFolder, fileName );
					paths.Insert( finalPath );
					names.Insert( fileName );
				}
			}
		}

		CloseFile( handle );
	}

	private string ExtractAttribute( string line, string attribute )
	{
		string marker = attribute + "=\"";
		int start = line.IndexOf( marker );
		if ( start == -1 )
			return "";

		start += marker.Length();
		int end = Mu_XML_IndexOfFrom( line, "\"", start );
		if ( end == -1 )
			return "";

		return line.Substring( start, end - start );
	}

	// ------------------------------------------------------------------
	// 2) Lista typow w wybranym pliku
	// ------------------------------------------------------------------

	void RequestTypesFromFile( string filePath )
	{
		if ( IsMissionHost() )
		{
			Exec_GetTypesFromFile( filePath, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( filePath );
			rpc.Send( NULL, JMXMLEditorModuleRPC.GetTypesFromFile, true, NULL );
		}
	}

	private void RPC_GetTypesFromFile( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string filePath;
		if ( !ctx.Read( filePath ) )
			return;

		Exec_GetTypesFromFile( filePath, senderRPC );
	}

	private void Exec_GetTypesFromFile( string filePath, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.View", ident, instance ) )
			return;

		JMXMLParser parser = new JMXMLParser( filePath );
		if ( !parser.Load() || !parser.VerifyXml() )
		{
			Print( "[Mu_XMLEditor] Nie udalo sie wczytac pliku: " + filePath );
			return;
		}

		array<string> typeNames = parser.GetAllTypeNames();

		if ( ident != NULL && typeNames.Count() > 0 )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( typeNames );
			rpc.Send( NULL, JMXMLEditorModuleRPC.HandleTypesList, true, ident );
		}
	}

	// ------------------------------------------------------------------
	// 3) Szczegoly konkretnego typu - 7 stalych pol w ustalonej kolejnosci:
	//    nominal, lifetime, restock, min, quantmin, quantmax, cost
	// ------------------------------------------------------------------

	void RequestDetails( string filePath, string itemType )
	{
		if ( IsMissionHost() )
		{
			Exec_GetDetails( filePath, itemType, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( filePath );
			rpc.Write( itemType );
			rpc.Send( NULL, JMXMLEditorModuleRPC.GetDetails, true, NULL );
		}
	}

	private void RPC_GetDetails( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string filePath, itemType;
		if ( !ctx.Read( filePath ) )
			return;
		if ( !ctx.Read( itemType ) )
			return;

		Exec_GetDetails( filePath, itemType, senderRPC );
	}

	private void Exec_GetDetails( string filePath, string itemType, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.View", ident, instance ) )
			return;

		JMXMLParser parser = new JMXMLParser( filePath );
		if ( !parser.Load() || !parser.VerifyXml() )
			return;

		array<ref JMXMLLineElement> scope = parser.GetElementString( "type", itemType );

		string vNominal  = ReadFieldOrEmpty( parser, "nominal",  scope );
		string vLifetime = ReadFieldOrEmpty( parser, "lifetime", scope );
		string vRestock  = ReadFieldOrEmpty( parser, "restock",  scope );
		string vMin      = ReadFieldOrEmpty( parser, "min",      scope );
		string vQuantMin = ReadFieldOrEmpty( parser, "quantmin", scope );
		string vQuantMax = ReadFieldOrEmpty( parser, "quantmax", scope );
		string vCost     = ReadFieldOrEmpty( parser, "cost",     scope );

		SendWebhook( "Scan", instance, "Otworzyl edycje typu: " + itemType + " (" + filePath + ")" );

		if ( ident != NULL )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( itemType );
			rpc.Write( vNominal );
			rpc.Write( vLifetime );
			rpc.Write( vRestock );
			rpc.Write( vMin );
			rpc.Write( vQuantMin );
			rpc.Write( vQuantMax );
			rpc.Write( vCost );
			rpc.Send( NULL, JMXMLEditorModuleRPC.HandleDetails, true, ident );
		}
	}

	private string ReadFieldOrEmpty( JMXMLParser parser, string elementType, array<ref JMXMLLineElement> scope )
	{
		JMXMLLineElement elementData = parser.GetElementStringValue( elementType, scope );
		if ( elementData == null )
			return "";

		return elementData.data;
	}

	// ------------------------------------------------------------------
	// 4) Zapis zmian - 7 stalych pol w tej samej kolejnosci co GetDetails
	// ------------------------------------------------------------------

	void SubmitChanges( string filePath, string itemType, string nominal, string lifetime, string restock, string min, string quantmin, string quantmax, string cost )
	{
		if ( IsMissionHost() )
		{
			Exec_SaveChanges( filePath, itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( filePath );
			rpc.Write( itemType );
			rpc.Write( nominal );
			rpc.Write( lifetime );
			rpc.Write( restock );
			rpc.Write( min );
			rpc.Write( quantmin );
			rpc.Write( quantmax );
			rpc.Write( cost );
			rpc.Send( NULL, JMXMLEditorModuleRPC.SaveChanges, true, NULL );
		}
	}

	private void RPC_SaveChanges( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string filePath, itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost;

		if ( !ctx.Read( filePath ) )  return;
		if ( !ctx.Read( itemType ) )  return;
		if ( !ctx.Read( nominal ) )   return;
		if ( !ctx.Read( lifetime ) )  return;
		if ( !ctx.Read( restock ) )   return;
		if ( !ctx.Read( min ) )       return;
		if ( !ctx.Read( quantmin ) )  return;
		if ( !ctx.Read( quantmax ) )  return;
		if ( !ctx.Read( cost ) )      return;

		Exec_SaveChanges( filePath, itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost, senderRPC );
	}

	private void Exec_SaveChanges( string filePath, string itemType, string nominal, string lifetime, string restock, string min, string quantmin, string quantmax, string cost, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.Edit", ident, instance ) )
			return;

		JMXMLParser parser = new JMXMLParser( filePath );
		if ( !parser.Load() || !parser.VerifyXml() )
			return;

		array<ref JMXMLLineElement> scope = parser.GetElementString( "type", itemType );

		WriteFieldIfPresent( parser, "nominal",  scope, nominal );
		WriteFieldIfPresent( parser, "lifetime", scope, lifetime );
		WriteFieldIfPresent( parser, "restock",  scope, restock );
		WriteFieldIfPresent( parser, "min",      scope, min );
		WriteFieldIfPresent( parser, "quantmin", scope, quantmin );
		WriteFieldIfPresent( parser, "quantmax", scope, quantmax );
		WriteFieldIfPresent( parser, "cost",     scope, cost );

		CreateBackup( filePath );
		parser.SaveChanges( filePath );

		GetCommunityOnlineToolsBase().Log( ident, "[XMLEditor] Zapisano zmiany w " + itemType + " (" + filePath + ")" );
		SendWebhook( "Edit", instance, "Zapisal zmiany w typie: " + itemType + " (" + filePath + ")" );
	}

	private void WriteFieldIfPresent( JMXMLParser parser, string elementType, array<ref JMXMLLineElement> scope, string newValue )
	{
		JMXMLLineElement elementData = parser.GetElementStringValue( elementType, scope );
		if ( elementData != null && elementData.index > -1 )
		{
			parser.ReplaceElementValue( elementType, elementData.index, newValue );
		}
	}

	private void CreateBackup( string originalPath )
	{
		int hour, minute, second;
		int year, month, day;

		GetHourMinuteSecondUTC( hour, minute, second );
		GetYearMonthDayUTC( year, month, day );

		string fileName = originalPath;
		fileName.Replace( "$mission:", "" );
		fileName.Replace( "/", "_" );
		fileName.Replace( ".xml", "" );

		string timeStamp = string.Format( "%1-%2-%3_%4-%5-%6", year.ToString(), month.ToString(), day.ToString(), hour.ToString(), minute.ToString(), second.ToString() );

		MakeDirectory( XML_PATH_BACKUP );
		CopyFile( originalPath, XML_PATH_BACKUP + fileName + "_" + timeStamp + ".xml" );
	}

	// ------------------------------------------------------------------
	// 5) Usuwanie obiektu pod kursorem (np. zdublowany/zepsuty CE spawn)
	// ------------------------------------------------------------------

	// ------------------------------------------------------------------
	// 6) Skanowanie lootu danego typu w promieniu wokol admina - zwraca
	//    pozycje + liczbe znalezionych egzemplarzy (posortowane od najblizszego)
	// ------------------------------------------------------------------

	void RequestScanLoot( string itemType, float radius )
	{
		if ( IsMissionHost() )
		{
			Exec_ScanLoot( itemType, radius, NULL );
		}
		else
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( itemType );
			rpc.Write( radius );
			rpc.Send( NULL, JMXMLEditorModuleRPC.ScanLoot, true, NULL );
		}
	}

	private void RPC_ScanLoot( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		string itemType;
		float radius;

		if ( !ctx.Read( itemType ) ) return;
		if ( !ctx.Read( radius ) )   return;

		Exec_ScanLoot( itemType, radius, senderRPC );
	}

	private void Exec_ScanLoot( string itemType, float radius, PlayerIdentity ident )
	{
		JMPlayerInstance instance;
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.ScanLoot", ident, instance ) )
			return;

		if ( !instance || !instance.PlayerObject )
			return;

		PlayerBase admin = PlayerBase.Cast( instance.PlayerObject );
		if ( !admin )
			return;

		if ( radius <= 0 )
			radius = 800;

		vector center = admin.GetPosition();
		vector boxMin = center + Vector( -radius, -400, -radius );
		vector boxMax = center + Vector(  radius,  400,  radius );

		string searchType = itemType;
		searchType.ToLower();

		array<EntityAI> entities = new array<EntityAI>;
		DayZPlayerUtils.SceneGetEntitiesInBox( boxMin, boxMax, entities );

		array<vector> positions = new array<vector>;

		foreach ( EntityAI ent : entities )
		{
			if ( !ent )
				continue;

			string entType = ent.GetType();
			entType.ToLower();

			if ( entType == searchType )
				positions.Insert( ent.GetPosition() );
		}

		SortPositionsByDistance( positions, center );

		SendWebhook( "Scan", instance, "Przeskanowal loot typu: " + itemType + " (znaleziono " + positions.Count().ToString() + ")" );

		if ( ident != NULL )
		{
			ScriptRPC rpc = new ScriptRPC();
			rpc.Write( itemType );
			rpc.Write( positions );
			rpc.Send( NULL, JMXMLEditorModuleRPC.HandleScanResults, true, ident );
		}
	}

	// Prosta sortuj-przez-wstawianie wg odleglosci od admina - listy z reguly sa krotkie (loot w promieniu kilkuset m)
	private void SortPositionsByDistance( inout array<vector> positions, vector origin )
	{
		for ( int i = 1; i < positions.Count(); i++ )
		{
			vector key = positions.Get( i );
			float keyDist = vector.DistanceSq( key, origin );
			int j = i - 1;

			while ( j >= 0 && vector.DistanceSq( positions.Get( j ), origin ) > keyDist )
			{
				positions.Set( j + 1, positions.Get( j ) );
				j = j - 1;
			}

			positions.Set( j + 1, key );
		}
	}

	// ------------------------------------------------------------------
	// Odbior odpowiedzi na kliencie -> przekazanie do aktywnego formularza
	// ------------------------------------------------------------------

	private void RPC_HandleTypesFiles( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		array<string> paths, names;
		if ( !ctx.Read( paths ) )
			return;
		if ( !ctx.Read( names ) )
			return;

		JMXMLEditorForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.HandleTypesFiles( paths, names );
	}

	private void RPC_HandleTypesList( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		array<string> typeNames;
		if ( !ctx.Read( typeNames ) )
			return;

		JMXMLEditorForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.HandleTypesList( typeNames );
	}

	private void RPC_HandleDetails( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		string itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost;

		if ( !ctx.Read( itemType ) ) return;
		if ( !ctx.Read( nominal ) )  return;
		if ( !ctx.Read( lifetime ) ) return;
		if ( !ctx.Read( restock ) )  return;
		if ( !ctx.Read( min ) )      return;
		if ( !ctx.Read( quantmin ) ) return;
		if ( !ctx.Read( quantmax ) ) return;
		if ( !ctx.Read( cost ) )     return;

		JMXMLEditorForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.HandleDetails( itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost );
	}

	private void RPC_HandleScanResults( ParamsReadContext ctx, PlayerIdentity senderRPC, Object target )
	{
		if ( !IsMissionClient() )
			return;

		string itemType;
		array<vector> positions;

		if ( !ctx.Read( itemType ) )  return;
		if ( !ctx.Read( positions ) ) return;

		JMXMLEditorForm form;
		if ( Class.CastTo( form, GetForm() ) )
			form.HandleScanResults( itemType, positions );
	}
};
