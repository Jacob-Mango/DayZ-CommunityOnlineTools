/*
	COT_VehicleManager - interface (client)

	1) Bouton "Supprimer sur Spawn" dans le menu Vehicules de COT (existant).
	2) Trois boutons d'action dans le panneau de DETAILS d'un vehicule, inseres
	   ENTRE "TP au vehicule" et "Retour" :
	       - Retourner : remet a l'endroit un vehicule retourne.
	       - Reparer   : repare sans ajouter/retirer pieces ni loot.
	       - Restaurer : repare + remet toutes les pieces + fluides a niveau.

	Les boutons sont crees dynamiquement dans la grille d'options de COT
	(vehicle_info_buttons_spacer, 6 lignes), puis "Retour" est renvoye en bas
	pour obtenir l'ordre : Supprimer / TP / Retourner / Reparer / Restaurer / Retour.
*/

modded class JMVehiclesMenu
{
#ifdef EXPANSIONMODVEHICLE
	protected UIActionButton m_CVMButton;
#endif

	// Boutons d'action par vehicule
	protected ButtonWidget m_CVMFlipButton;
	protected TextWidget   m_CVMFlipLabel;
	protected ButtonWidget m_CVMRepairButton;
	protected TextWidget   m_CVMRepairLabel;
	protected ButtonWidget m_CVMRestoreButton;
	protected TextWidget   m_CVMRestoreLabel;

	override void OnInit()
	{
		super.OnInit();

		CVM_InitActionButtons();

#ifndef EXPANSIONMODVEHICLE
		if ( m_DeleteUnclaimedButton )
		{
			m_DeleteUnclaimedButton.Show( true );
			if ( m_DeleteUnclaimedButtonLabel )
				m_DeleteUnclaimedButtonLabel.SetText( Widget.TranslateString( "#STR_CVM_Title" ) );
		}
#else
		Widget btnPanel = layoutRoot.FindAnyWidget( "vehicles_list_buttons" );
		if ( btnPanel && !m_CVMButton )
		{
			m_CVMButton = UIActionManager.CreateButton( btnPanel, "#STR_CVM_Title", this, "OnClick_CVMButton" );
			if ( m_CVMButton )
				m_CVMButton.SetColor( COLOR_RED );
		}
#endif
	}

	// Cree les 3 boutons d'action dans la grille d'options du vehicule.
	protected void CVM_InitActionButtons()
	{
		if ( m_CVMFlipButton ) // deja initialise
			return;

		Widget grid = layoutRoot.FindAnyWidget( "vehicle_info_buttons_spacer" );
		if ( !grid )
			return;

		Widget root;

		root = g_Game.GetWorkspace().CreateWidgets( "COT_VehicleManager/GUI/cvm_action_button.layout", grid );
		if ( root )
		{
			m_CVMFlipButton = ButtonWidget.Cast( root );
			m_CVMFlipLabel  = TextWidget.Cast( root.FindAnyWidget( "cvm_action_button_label" ) );
			if ( m_CVMFlipLabel )
				m_CVMFlipLabel.SetText( Widget.TranslateString( "#STR_CVM_FLIP" ) );
		}

		root = g_Game.GetWorkspace().CreateWidgets( "COT_VehicleManager/GUI/cvm_action_button.layout", grid );
		if ( root )
		{
			m_CVMRepairButton = ButtonWidget.Cast( root );
			m_CVMRepairLabel  = TextWidget.Cast( root.FindAnyWidget( "cvm_action_button_label" ) );
			if ( m_CVMRepairLabel )
				m_CVMRepairLabel.SetText( Widget.TranslateString( "#STR_CVM_REPAIR" ) );
		}

		root = g_Game.GetWorkspace().CreateWidgets( "COT_VehicleManager/GUI/cvm_action_button.layout", grid );
		if ( root )
		{
			m_CVMRestoreButton = ButtonWidget.Cast( root );
			m_CVMRestoreLabel  = TextWidget.Cast( root.FindAnyWidget( "cvm_action_button_label" ) );
			if ( m_CVMRestoreLabel )
				m_CVMRestoreLabel.SetText( Widget.TranslateString( "#STR_CVM_RESTORE" ) );
		}

		// Renvoie "Retour" tout en bas pour que les 3 nouveaux boutons
		// se placent entre "TP au vehicule" et "Retour".
		if ( m_CancleVehicleEdit )
			grid.AddChild( m_CancleVehicleEdit, false );

		grid.Update();
	}

#ifndef EXPANSIONMODVEHICLE
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( CVM_HandleActionClick( w ) )
			return true;

		if ( w == m_DeleteUnclaimedButton ) // recycle -> "Supprimer sur Spawn"
		{
			CVM_RequestPreview();
			return true; // ne pas laisser COT ouvrir la confirmation "Unclaimed"
		}
		return super.OnClick( w, x, y, button );
	}
#else
	override bool OnClick( Widget w, int x, int y, int button )
	{
		if ( CVM_HandleActionClick( w ) )
			return true;

		return super.OnClick( w, x, y, button );
	}

	void OnClick_CVMButton( UIEvent eid, UIActionBase action )
	{
		if ( eid != UIEvent.CLICK )
			return;
		CVM_RequestPreview();
	}
#endif

	// Renvoie true si le clic correspond a un de nos boutons d'action.
	protected bool CVM_HandleActionClick( Widget w )
	{
		if ( w == m_CVMFlipButton )
		{
			CVM_DoVehicleAction( CVM_ACT_FLIP );
			return true;
		}
		if ( w == m_CVMRepairButton )
		{
			CVM_DoVehicleAction( CVM_ACT_REPAIR );
			return true;
		}
		if ( w == m_CVMRestoreButton )
		{
			CVM_DoVehicleAction( CVM_ACT_RESTORE );
			return true;
		}
		return false;
	}

	protected void CVM_DoVehicleAction( int action )
	{
		if ( !m_CurrentVehicle )
			return;

		GetRPCManager().SendRPC( "CVM_RPC", "RPC_CVMVehicleAction",
			new Param3<int, int, int>( m_CurrentVehicle.m_NetworkIDLow, m_CurrentVehicle.m_NetworkIDHigh, action ), true );
	}

	// Survol : texte en noir (coherent avec les boutons COT).
	override bool OnMouseEnter( Widget w, int x, int y )
	{
		if ( w == m_CVMFlipButton && m_CVMFlipLabel )
			m_CVMFlipLabel.SetColor( ARGB( 255, 0, 0, 0 ) );
		else if ( w == m_CVMRepairButton && m_CVMRepairLabel )
			m_CVMRepairLabel.SetColor( ARGB( 255, 0, 0, 0 ) );
		else if ( w == m_CVMRestoreButton && m_CVMRestoreLabel )
			m_CVMRestoreLabel.SetColor( ARGB( 255, 0, 0, 0 ) );
		else
			return super.OnMouseEnter( w, x, y );

		return false;
	}

	override bool OnMouseLeave( Widget w, Widget enterW, int x, int y )
	{
		if ( w == m_CVMFlipButton && m_CVMFlipLabel )
			m_CVMFlipLabel.SetColor( ARGB( 255, 255, 255, 255 ) );
		else if ( w == m_CVMRepairButton && m_CVMRepairLabel )
			m_CVMRepairLabel.SetColor( ARGB( 255, 255, 255, 255 ) );
		else if ( w == m_CVMRestoreButton && m_CVMRestoreLabel )
			m_CVMRestoreLabel.SetColor( ARGB( 255, 255, 255, 255 ) );
		else
			return super.OnMouseLeave( w, enterW, x, y );

		return false;
	}

	// ================= Nettoyage sur spawn (existant) =================
	void CVM_RequestPreview()
	{
		GetRPCManager().SendRPC( "CVM_RPC", "RPC_CVMCleanVehiclesOnSpawn", new Param2<float, int>( CVM_RADIUS, CVM_MODE_PREVIEW ), true );
	}

	// Appele par le module a la reception du resultat serveur.
	void CVM_ApplyPreview( int scanned, int radius, array<vector> matchedPositions )
	{
		int matched = matchedPositions.Count();

		// Recolore les marqueurs : rouge = sera supprime, vert = conserve.
		int red   = ARGB( 255, 220, 40, 40 );
		int green = ARGB( 255, 40, 200, 80 );

		foreach ( JMVehiclesMapMarker m : m_MapMarkers )
		{
			if ( !m )
				continue;

			if ( CVM_PosInList( m.CVM_GetPos(), matchedPositions ) )
				m.ChangeColor( red );
			else
				m.ChangeColor( green );
		}

		string head = string.Format( Widget.TranslateString( "#STR_CVM_Count" ), matched.ToString(), scanned.ToString(), radius.ToString() );

		if ( matched <= 0 )
		{
			CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_CVM_Title", head + "\n" + Widget.TranslateString( "#STR_CVM_Nothing" ), "#STR_COT_GENERIC_CANCEL", "", "OK", "CVM_Noop" );
			return;
		}

		string body = head + "\n" + Widget.TranslateString( "#STR_CVM_Legend" ) + "\n" + Widget.TranslateString( "#STR_CVM_ConfirmQ" );

		// (type, titre, message, texteBtn1, cbBtn1, texteBtn2, cbBtn2)
		CreateConfirmation_Two( JMConfirmationType.INFO, "#STR_CVM_Title", body, "#STR_COT_GENERIC_CANCEL", "", "#STR_COT_OBJECT_MODULE_DELETE", "CVM_Clean_Yes" );
	}

	private bool CVM_PosInList( vector p, array<vector> list )
	{
		foreach ( vector q : list )
		{
			float dx = p[0] - q[0];
			float dz = p[2] - q[2];
			if ( ( dx * dx + dz * dz ) <= 1.0 ) // tolerance 1 m
				return true;
		}
		return false;
	}

	void CVM_Clean_Yes( JMConfirmation confirmation )
	{
		GetRPCManager().SendRPC( "CVM_RPC", "RPC_CVMCleanVehiclesOnSpawn", new Param2<float, int>( CVM_RADIUS, CVM_MODE_DELETE ), true );
	}

	void CVM_Noop( JMConfirmation confirmation )
	{
	}

	// ===== Joueurs sur la carte + teleport double-clic (comme les autres addons) =====
	override void OnShow()
	{
		super.OnShow();
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).CallLater( CVM_UpdatePlayers, 1000, true );
	}

	override void OnHide()
	{
		super.OnHide();
		g_Game.GetCallQueue( CALL_CATEGORY_GUI ).Remove( CVM_UpdatePlayers );
	}

	// Affiche la position des joueurs (points bleus). Les marqueurs vehicules de COT
	// sont des widgets separes, donc ClearUserMarks ne les efface pas.
	void CVM_UpdatePlayers()
	{
		if ( !m_MapWidget )
			return;

		m_MapWidget.ClearUserMarks();

		array<JMPlayerInstance> players = GetPermissionsManager().GetPlayers();
		foreach ( JMPlayerInstance player : players )
		{
			m_MapWidget.AddUserMark( player.GetPosition(), player.GetName(), ARGB( 255, 52, 152, 219 ), JM_COT_ICON_DOT + ".paa" );
		}

		GetCommunityOnlineTools().RefreshClientPositions();
	}

	// Double-clic sur la carte -> teleportation de l'admin a l'endroit clique.
	override bool OnDoubleClick( Widget w, int x, int y, int button )
	{
		if ( w && w == m_MapWidget )
		{
			vector tpPos = SnapToGround( m_MapWidget.ScreenToMap( Vector( x, y, 0 ) ) );
			string guid = GetPermissionsManager().GetClientPlayer().GetGUID();
			CF_Modules<JMTeleportModule>.Get().Position( tpPos, { guid } );
			return true;
		}

		return false;
	}
}
