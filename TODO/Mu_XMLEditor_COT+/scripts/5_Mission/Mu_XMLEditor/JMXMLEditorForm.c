/*
	JMXMLEditorForm
	-----------------
	Interfejs uzytkownika modulu JMXMLEditorModule, zbudowany w 100%
	przy pomocy natywnego UIActionManager z COT (tak jak wszystkie inne
	formularze COT - patrz JMExampleForm dla wzorca).
*/
class JMXMLEditorForm : JMFormBase
{
	// Wartosc sentinel w m_FilePaths oznaczajaca "wszystkie pliki naraz"
	static const string ALL_FILES_KEY = "__ALL__";

	protected UIActionScroller m_Scroller;

	protected UIActionSelectBox  m_FileSelect;
	protected ref array<string>  m_FilePaths        = new array<string>;
	protected ref array<string>  m_FileDisplayNames = new array<string>;

	protected UIActionEditableText  m_TypeSearch;
	protected UIActionButton        m_BtnSearchConfirm;
	protected UIActionSelectBox     m_TypeSelect;
	protected ref array<string>     m_TypeNamesAll      = new array<string>; // pelna lista (wszystkie pliki)
	protected ref array<string>     m_TypeNamesFiltered = new array<string>; // aktualna wyswietlana lista po filtrze
	protected ref array<int>        m_FilteredToAll     = new array<int>;    // mapowanie: index filtrowanej -> index w m_TypeNamesAll

	protected UIActionText           m_StatusText;

	protected UIActionEditableText   m_FieldNominal;
	protected UIActionEditableText   m_FieldLifetime;
	protected UIActionEditableText   m_FieldRestock;
	protected UIActionEditableText   m_FieldMin;
	protected UIActionEditableText   m_FieldQuantMin;
	protected UIActionEditableText   m_FieldQuantMax;
	protected UIActionEditableText   m_FieldCost;

	protected UIActionButton m_BtnRefresh;
	protected UIActionButton m_BtnSave;

	protected ItemPreviewWidget m_ItemPreview3D;
	protected EntityAI          m_PreviewItem;
	protected UIActionText       m_ItemDisplayName;

	void ~JMXMLEditorForm()
	{
		if ( m_PreviewItem )
			g_Game.ObjectDelete( m_PreviewItem );
	}

	protected UIActionEditableText m_FieldScanRadius;
	protected UIActionButton       m_BtnScanLoot;
	protected UIActionText         m_ScanCountText;
	protected UIActionText         m_ScanNearestText;
	protected UIActionButton       m_BtnTeleportNearest;

	protected JMXMLEditorModule m_Module;

	protected ref array<string> m_PendingChanges; // [nominal,lifetime,restock,min,quantmin,quantmax,cost]
	protected ref array<vector> m_LastScanPositions;

	protected override bool SetModule( JMRenderableModuleBase mdl )
	{
		bool result = Class.CastTo( m_Module, mdl );
		Print( "[Mu_XMLEditor][DEBUG] JMXMLEditorForm::SetModule() wynik castowania = " + result.ToString() );
		return result;
	}

	override void OnInit()
	{
		Print( "[Mu_XMLEditor][DEBUG] JMXMLEditorForm::OnInit() wywolane - buduje GUI." );

		m_Scroller = UIActionManager.CreateScroller( layoutRoot.FindAnyWidget( "panel" ) );
		Widget actions = m_Scroller.GetContentWidget();

		// Podglad 3D - pobieramy widget bezposrednio z layoutu (identycznie jak JMObjectSpawnerForm)
		m_ItemPreview3D = ItemPreviewWidget.Cast( layoutRoot.FindAnyWidget( "item_preview_3d" ) );

		m_StatusText = UIActionManager.CreateText( actions, "Status:", "Wybierz plik types.xml" );

		GridSpacerWidget filePanel = UIActionManager.CreateGridSpacer( actions, 2, 1 );
		m_FileSelect = UIActionManager.CreateSelectionBox( filePanel, "Plik:", m_FileDisplayNames, this, "OnChange_FileSelect" );
		m_BtnRefresh = UIActionManager.CreateButton( filePanel, "Odswiez liste plikow", this, "OnClick_Refresh" );

		GridSpacerWidget searchPanel = UIActionManager.CreateGridSpacer( actions, 2, 1 );
		m_TypeSearch        = UIActionManager.CreateEditableText( searchPanel, "Szukaj itemu:", this, "OnChange_TypeSearch" );
		m_BtnSearchConfirm  = UIActionManager.CreateButton( searchPanel, "Zaladuj", this, "OnClick_SearchConfirm" );

		m_TypeSelect = UIActionManager.CreateSelectionBox( actions, "Typ przedmiotu:", m_TypeNamesFiltered, this, "OnChange_TypeSelect" );

		// Pola edycji wartosci types.xml
		GridSpacerWidget fieldsPanel = UIActionManager.CreateGridSpacer( actions, 7, 1 );
		m_FieldNominal  = UIActionManager.CreateEditableText( fieldsPanel, "nominal:",   this, "OnChange_Field" );
		m_FieldLifetime = UIActionManager.CreateEditableText( fieldsPanel, "lifetime:",  this, "OnChange_Field" );
		m_FieldRestock  = UIActionManager.CreateEditableText( fieldsPanel, "restock:",   this, "OnChange_Field" );
		m_FieldMin      = UIActionManager.CreateEditableText( fieldsPanel, "min:",       this, "OnChange_Field" );
		m_FieldQuantMin = UIActionManager.CreateEditableText( fieldsPanel, "quantmin:",  this, "OnChange_Field" );
		m_FieldQuantMax = UIActionManager.CreateEditableText( fieldsPanel, "quantmax:",  this, "OnChange_Field" );
		m_FieldCost     = UIActionManager.CreateEditableText( fieldsPanel, "cost:",      this, "OnChange_Field" );

		m_ItemDisplayName = UIActionManager.CreateText( actions, "Nazwa:", "-" );

		m_BtnSave = UIActionManager.CreateButton( actions, "Zapisz zmiany", this, "OnClick_Save" );

		// --- Skanowanie lootu (pozycja + ilosc wybranego typu w promieniu wokol admina) ---
		GridSpacerWidget scanPanel = UIActionManager.CreateGridSpacer( actions, 2, 1 );
		m_FieldScanRadius = UIActionManager.CreateEditableText( scanPanel, "Promien (m):", this, "OnChange_Field" );
		m_FieldScanRadius.SetText( "800" );
		m_BtnScanLoot     = UIActionManager.CreateButton( scanPanel, "Skanuj loot na mapie", this, "OnClick_ScanLoot" );

		m_ScanCountText   = UIActionManager.CreateText( actions, "Wyniki skanu:", "-" );
		m_ScanNearestText = UIActionManager.CreateText( actions, "Najblizszy:", "-" );

		m_BtnTeleportNearest = UIActionManager.CreateButton( actions, "Teleportuj do najblizszego", this, "OnClick_TeleportNearest" );
		m_BtnTeleportNearest.Disable();

		SetFieldsEnabled( false );
	}

	override void OnShow()
	{
		super.OnShow();

		if ( m_FileDisplayNames.Count() == 0 )
			RefreshFiles();
	}

	override void OnHide()
	{
		super.OnHide();
	}

	override void OnSettingsUpdated()
	{
	}

	// ------------------------------------------------------------------
	// Pliki types.xml
	// ------------------------------------------------------------------

	void OnClick_Refresh( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		RefreshFiles();
	}

	void RefreshFiles()
	{
		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.View" ) )
			return;

		m_StatusText.SetText( "Wczytywanie listy plikow..." );
		m_Module.RequestTypesFiles();
	}

	// Wywolywane przez RPC po stronie klienta po odebraniu odpowiedzi serwera
	void HandleTypesFiles( array<string> paths, array<string> displayNames )
	{
		m_FilePaths.Clear();
		m_FileDisplayNames.Clear();

		// Zakładka "Wszystkie" – pierwsza pozycja na liście
		m_FilePaths.Insert( ALL_FILES_KEY );
		m_FileDisplayNames.Insert( "--- Wszystkie ---" );

		foreach ( string p : paths )
			m_FilePaths.Insert( p );

		foreach ( string n : displayNames )
			m_FileDisplayNames.Insert( n );

		m_FileSelect.SetSelections( m_FileDisplayNames );
		m_StatusText.SetText( "Wybierz plik i typ przedmiotu" );
	}

	void OnChange_FileSelect( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		int idx = m_FileSelect.GetSelection();
		if ( idx < 0 || idx >= m_FilePaths.Count() )
			return;

		SetFieldsEnabled( false );
		m_TypeSearch.SetText( "" );
		m_TypeNamesAll.Clear();
		m_TypeNamesFiltered.Clear();
		m_FilteredToAll.Clear();
		m_TypeSelect.SetSelections( m_TypeNamesFiltered );

		string selectedPath = m_FilePaths.Get( idx );

		if ( selectedPath == ALL_FILES_KEY )
		{
			// Poproś serwer o typy ze WSZYSTKICH plików po kolei –
			// dla każdego pliku (poza sentinel) wysyłamy osobne zapytanie.
			// Serwer odpowie kilkukrotnie HandleTypesList; łączymy wyniki.
			m_StatusText.SetText( "Wczytywanie typow ze wszystkich plikow..." );
			m_PendingAllFilesCount = m_FilePaths.Count() - 1; // bez sentinel
			m_ReceivedAllFilesCount = 0;
			for ( int i = 1; i < m_FilePaths.Count(); i++ )
				m_Module.RequestTypesFromFile( m_FilePaths.Get( i ) );
		}
		else
		{
			m_PendingAllFilesCount = 0;
			m_StatusText.SetText( "Wczytywanie listy typow..." );
			m_Module.RequestTypesFromFile( selectedPath );
		}
	}

	// ------------------------------------------------------------------
	// Typy przedmiotow
	// ------------------------------------------------------------------

	// Liczniki do scalania wielu odpowiedzi dla "Wszystkie"
	protected int m_PendingAllFilesCount  = 0;
	protected int m_ReceivedAllFilesCount = 0;

	void HandleTypesList( array<string> typeNames )
	{
		m_ReceivedAllFilesCount++;

		foreach ( string t : typeNames )
		{
			// unikamy duplikatow przy scalaniu wielu plikow
			if ( m_TypeNamesAll.Find( t ) == -1 )
				m_TypeNamesAll.Insert( t );
		}

		// Czekamy az przyjda odpowiedzi ze wszystkich plikow (lub jest to pojedynczy plik)
		bool allReceived = ( m_PendingAllFilesCount == 0 || m_ReceivedAllFilesCount >= m_PendingAllFilesCount );

		if ( allReceived )
		{
			m_TypeSearch.SetText( "" );
			RebuildFilteredList( "" );
			m_StatusText.SetText( "Znaleziono " + m_TypeNamesAll.Count().ToString() + " typow. Wyszukaj lub wybierz." );
		}
	}

	// Filtrowanie: buduje m_TypeNamesFiltered + m_FilteredToAll z biezacym zapytaniem
	void RebuildFilteredList( string query )
	{
		m_TypeNamesFiltered.Clear();
		m_FilteredToAll.Clear();

		string q = query;
		q.ToLower();

		for ( int i = 0; i < m_TypeNamesAll.Count(); i++ )
		{
			string name = m_TypeNamesAll.Get( i );
			string nameLower = name;
			nameLower.ToLower();

			if ( q == "" || nameLower.IndexOf( q ) != -1 )
			{
				m_TypeNamesFiltered.Insert( name );
				m_FilteredToAll.Insert( i );
			}
		}

		m_TypeSelect.SetSelections( m_TypeNamesFiltered );
	}

	void OnChange_TypeSearch( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CHANGE )
			return;

		RebuildFilteredList( m_TypeSearch.GetText() );
	}

	void OnClick_SearchConfirm( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( m_TypeNamesFiltered.Count() > 0 )
		{
			m_TypeSelect.SetSelection( 0 );
			LoadDetailsForSelectedType();
		}
	}

	void OnChange_TypeSelect( UIEvent eid, UIActionBase action )
	{
		// Reagujemy zarowno na zmiane strzalkami (CHANGE) jak i klikniecie (CLICK)
		if ( eid != UIEvent.CHANGE && eid != UIEvent.CLICK )
			return;

		LoadDetailsForSelectedType();
	}

	// Wspolna logika ladowania danych dla aktualnie zaznaczonego itemu
	protected void LoadDetailsForSelectedType()
	{
		int fileIdx = m_FileSelect.GetSelection();
		int typeIdx = m_TypeSelect.GetSelection();

		if ( fileIdx < 0 || typeIdx < 0 || typeIdx >= m_TypeNamesFiltered.Count() )
			return;

		string itemType = m_TypeNamesFiltered.Get( typeIdx );
		string selectedPath = m_FilePaths.Get( fileIdx );

		if ( selectedPath == ALL_FILES_KEY )
		{
			m_PendingDetailSearch = itemType;
			m_PendingDetailFileIdx = 1;
			TryNextFileForDetail();
			return;
		}

		m_PendingDetailSearch = "";
		m_StatusText.SetText( "Wczytywanie danych przedmiotu..." );
		m_Module.RequestDetails( selectedPath, itemType );
	}

	// Pomocnicze dla trybu "Wszystkie" – iteruje po plikach az znajdzie item
	protected string m_PendingDetailSearch = "";
	protected int    m_PendingDetailFileIdx = 1;

	protected void TryNextFileForDetail()
	{
		if ( m_PendingDetailFileIdx >= m_FilePaths.Count() )
		{
			m_StatusText.SetText( "Nie znaleziono: " + m_PendingDetailSearch );
			m_PendingDetailSearch = "";
			return;
		}

		m_StatusText.SetText( "Szukam w pliku " + m_PendingDetailFileIdx.ToString() + "/" + ( m_FilePaths.Count() - 1 ).ToString() + "..." );
		m_Module.RequestDetails( m_FilePaths.Get( m_PendingDetailFileIdx ), m_PendingDetailSearch );
	}

	// ------------------------------------------------------------------
	// Szczegoly / edycja
	// ------------------------------------------------------------------

	void HandleDetails( string itemType, string nominal, string lifetime, string restock, string min, string quantmin, string quantmax, string cost )
	{
		// Tryb "Wszystkie": jesli nominal jest pusty, item nie byl w tym pliku - probujemy nastepny
		if ( m_PendingDetailSearch != "" )
		{
			if ( nominal == "" )
			{
				m_PendingDetailFileIdx++;
				TryNextFileForDetail();
				return;
			}
			// Znaleziony – zerujemy stan szukania
			m_PendingDetailSearch = "";
		}

		m_FieldNominal.SetText( nominal );
		m_FieldLifetime.SetText( lifetime );
		m_FieldRestock.SetText( restock );
		m_FieldMin.SetText( min );
		m_FieldQuantMin.SetText( quantmin );
		m_FieldQuantMax.SetText( quantmax );
		m_FieldCost.SetText( cost );

		SetFieldsEnabled( true );
		m_StatusText.SetText( "Edytujesz: " + itemType );

		// Podglad ikony i nazwy wyswietlanej z configa gry
		UpdateItemPreview( itemType );
	}

	protected void UpdateItemPreview( string itemType )
	{
		// Usun poprzedni podglad
		if ( m_PreviewItem )
		{
			g_Game.ObjectDelete( m_PreviewItem );
			m_PreviewItem = null;
		}

		// Nazwa wyswietlana z configa
		string displayName = "";
		GetGame().ConfigGetText( "CfgVehicles " + itemType + " displayName", displayName );
		if ( displayName == "" )
			displayName = itemType;
		m_ItemDisplayName.SetText( displayName );

		if ( !m_ItemPreview3D )
			return;

		// Tworzenie lokalnego (klienckie, bez sieci) obiektu do podgladu 3D
		// - identyczny wzorzec jak JMObjectSpawnerForm.c z COT
		m_PreviewItem = EntityAI.Cast( g_Game.CreateObject( itemType, vector.Zero, true, false, false ) );

		if ( !m_PreviewItem )
		{
			m_ItemPreview3D.Show( false );
			return;
		}

		dBodyActive( m_PreviewItem, ActiveState.INACTIVE );
		m_PreviewItem.DisableSimulation( true );

		m_ItemPreview3D.SetItem( m_PreviewItem );
		m_ItemPreview3D.SetModelPosition( Vector( 0, 0, 0.5 ) );
		m_ItemPreview3D.SetModelOrientation( vector.Zero );
		m_ItemPreview3D.SetView( m_PreviewItem.GetViewIndex() );
		m_ItemPreview3D.Show( true );
	}

	void OnChange_Field( UIEvent eid, UIActionBase action )
	{
		// Tylko podglad na biezaco, faktyczny zapis nastepuje po kliknieciu "Zapisz zmiany"
	}

	void OnClick_Save( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.Edit" ) )
			return;

		int fileIdx = m_FileSelect.GetSelection();
		int typeIdx = m_TypeSelect.GetSelection();
		if ( fileIdx < 0 || typeIdx < 0 )
			return;

		m_PendingChanges = new array<string>;
		m_PendingChanges.Insert( m_FieldNominal.GetText() );
		m_PendingChanges.Insert( m_FieldLifetime.GetText() );
		m_PendingChanges.Insert( m_FieldRestock.GetText() );
		m_PendingChanges.Insert( m_FieldMin.GetText() );
		m_PendingChanges.Insert( m_FieldQuantMin.GetText() );
		m_PendingChanges.Insert( m_FieldQuantMax.GetText() );
		m_PendingChanges.Insert( m_FieldCost.GetText() );

		// Kolejnosc argumentow CreateConfirmation_Two:
		// (type, title, message, button1Title, button2Title, button1Callback, button2Callback)
		CreateConfirmation_Two( JMConfirmationType.INFO, "Zapisac zmiany?", "Plik zostanie nadpisany (backup zostanie utworzony automatycznie)", "#STR_COT_GENERIC_YES", "#STR_COT_GENERIC_NO", "OnConfirmSave", "" );
	}

	void OnConfirmSave()
	{
		int fileIdx = m_FileSelect.GetSelection();
		int typeIdx = m_TypeSelect.GetSelection();
		if ( fileIdx < 0 || typeIdx < 0 || m_PendingChanges == null )
			return;

		string filePath = m_FilePaths.Get( fileIdx );
		string itemType = m_TypeNamesFiltered.Get( typeIdx );

		// W trybie "Wszystkie" znamy plik zrodlowy bo iteracja przez TryNextFileForDetail
		// zatrzymala sie na m_PendingDetailFileIdx-1 (juz zdekrementowalismy po znalezieniu)
		// Bezpieczniej: jesli sentinel, cofamy sie do ostatniego pliku, gdzie znalezlismy item
		if ( filePath == ALL_FILES_KEY )
		{
			// m_PendingDetailFileIdx jest >=1 i wskazuje na plik po ostatnim nieudanym
			// (lub 1 jesli znalazl za pierwszym razem). Cofamy sie o 1.
			int srcIdx = m_PendingDetailFileIdx - 1;
			if ( srcIdx < 1 ) srcIdx = 1;
			filePath = m_FilePaths.Get( srcIdx );
		}

		string nominal  = m_PendingChanges.Get(0);
		string lifetime = m_PendingChanges.Get(1);
		string restock  = m_PendingChanges.Get(2);
		string min      = m_PendingChanges.Get(3);
		string quantmin = m_PendingChanges.Get(4);
		string quantmax = m_PendingChanges.Get(5);
		string cost     = m_PendingChanges.Get(6);

		m_Module.SubmitChanges( filePath, itemType, nominal, lifetime, restock, min, quantmin, quantmax, cost );

		m_StatusText.SetText( "Zapisano." );
		COTCreateLocalAdminNotification( new StringLocaliser( "XML Editor: zmiany zapisane" ) );
	}

	// ------------------------------------------------------------------
	// Skanowanie lootu - pozycja + ilosc wybranego typu wokol admina
	// ------------------------------------------------------------------

	void OnClick_ScanLoot( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !GetPermissionsManager().HasPermission( "Admin.XMLEditor.ScanLoot" ) )
			return;

		int typeIdx = m_TypeSelect.GetSelection();
		if ( typeIdx < 0 || typeIdx >= m_TypeNamesFiltered.Count() )
		{
			m_StatusText.SetText( "Najpierw wybierz typ przedmiotu z listy." );
			return;
		}

		string itemType = m_TypeNamesFiltered.Get( typeIdx );

		float radius = m_FieldScanRadius.GetText().ToFloat();
		if ( radius <= 0 )
			radius = 800;

		m_ScanCountText.SetText( "Skanowanie..." );
		m_ScanNearestText.SetText( "-" );
		m_BtnTeleportNearest.Disable();
		m_LastScanPositions = null;
		JMXMLEditorMapMarkers.Clear();

		m_Module.RequestScanLoot( itemType, radius );
	}

	// Wywolywane przez RPC po stronie klienta po odebraniu wynikow skanu
	void HandleScanResults( string itemType, array<vector> positions )
	{
		m_LastScanPositions = new array<vector>;
		foreach ( vector pos : positions )
			m_LastScanPositions.Insert( pos );

		if ( m_LastScanPositions.Count() == 0 )
		{
			m_ScanCountText.SetText( "Brak wynikow dla: " + itemType );
			m_ScanNearestText.SetText( "-" );
			m_BtnTeleportNearest.Disable();
			JMXMLEditorMapMarkers.Clear();
			return;
		}

		vector nearest = m_LastScanPositions.Get( 0 );
		m_ScanCountText.SetText( "Znaleziono: " + m_LastScanPositions.Count().ToString() + " szt." );
		m_ScanNearestText.SetText( nearest.ToString() );
		m_BtnTeleportNearest.Enable();

		// Wyslij znaczniki na istniejaca mape COT i ja otworz
		JMXMLEditorMapMarkers.SetMarkers( itemType, m_LastScanPositions );

		JMMapModule mapModule;
		if ( Class.CastTo( mapModule, GetModuleManager().GetModule( JMMapModule ) ) )
			mapModule.Show();
	}

	void OnClick_TeleportNearest( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;

		if ( !m_LastScanPositions || m_LastScanPositions.Count() == 0 )
			return;

		PlayerBase player = PlayerBase.Cast( g_Game.GetPlayer() );
		if ( !player || !player.GetIdentity() )
			return;

		JMPlayerModule playerModule;
		if ( !Class.CastTo( playerModule, GetModuleManager().GetModule( JMPlayerModule ) ) )
			return;

		vector nearest = m_LastScanPositions.Get( 0 );
		playerModule.TeleportTo( nearest, { player.GetIdentity().GetId() } );
	}

	// ------------------------------------------------------------------
	// Pomocnicze
	// ------------------------------------------------------------------

	protected void SetFieldsEnabled( bool enabled )
	{
		if ( !enabled )
		{
			if ( m_ItemPreview3D )
				m_ItemPreview3D.Show( false );
			if ( m_PreviewItem )
			{
				g_Game.ObjectDelete( m_PreviewItem );
				m_PreviewItem = null;
			}
			m_ItemDisplayName.SetText( "-" );
		}
		if ( enabled )
		{
			m_FieldNominal.Enable();
			m_FieldLifetime.Enable();
			m_FieldRestock.Enable();
			m_FieldMin.Enable();
			m_FieldQuantMin.Enable();
			m_FieldQuantMax.Enable();
			m_FieldCost.Enable();
			m_BtnSave.Enable();
		}
		else
		{
			m_FieldNominal.Disable();
			m_FieldLifetime.Disable();
			m_FieldRestock.Disable();
			m_FieldMin.Disable();
			m_FieldQuantMin.Disable();
			m_FieldQuantMax.Disable();
			m_FieldCost.Disable();
			m_BtnSave.Disable();
		}
	}
}
