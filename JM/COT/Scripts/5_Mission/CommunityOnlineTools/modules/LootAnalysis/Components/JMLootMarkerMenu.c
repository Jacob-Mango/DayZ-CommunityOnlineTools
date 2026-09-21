//! Right-click menu on a map marker - Teleport To / Copy Position / Center Map Here / Copy Name. One instance serves both of the form's maps, since the action set is identical either way.
class JMLootMarkerMenu
{
	protected JMLootAnalysisForm m_Form;

	//! Marker right-click menu rows.
	static const string MENU_MK_TELEPORT = "mk_teleport";
	static const string MENU_MK_COPYPOS  = "mk_copypos";
	static const string MENU_MK_FOCUS    = "mk_focus";
	static const string MENU_MK_COPYNAME = "mk_copyname";

	//! Right-click marker menu - shared by both maps, since the action set is
	//! identical either way.
	protected UIActionContextMenu m_MarkerMenu;
	protected UIActionMap m_MarkerMenuMap;
	protected vector m_MarkerMenuPos;
	protected string m_MarkerMenuLabel;

	void JMLootMarkerMenu( JMLootAnalysisForm form )
	{
		m_Form = form;
	}

	//! fallbackLabel names a marker that carries no on-map label of its own (distribution markers keep
	//! their name in a side table on the tab, which is where the form looks it up).
	void Show(UIActionMap clickedMap, string markerId, string fallbackLabel)
	{
		JMMapMarker marker = clickedMap.GetMarker(markerId);
		if (!marker)
			return;

		if (!m_MarkerMenu)
		{
			if (!m_Form.GetWindow())
				return;

			m_MarkerMenu = UIActionManager.CreateOverlayMenu( m_Form, this, "OnClick_MarkerMenu" );

			if (!m_MarkerMenu)
				return;

			m_Form.AddOverlay(m_MarkerMenu);
		}

		m_MarkerMenuMap   = clickedMap;
		m_MarkerMenuPos   = marker.Position;
		m_MarkerMenuLabel = marker.Label;

		if (m_MarkerMenuLabel == "")
			m_MarkerMenuLabel = fallbackLabel;

		m_MarkerMenu.ClearItems();
		m_MarkerMenu.AddItem(MENU_MK_TELEPORT, "Teleport To", JMConstants.Lucide("footprints"));
		m_MarkerMenu.AddItem(MENU_MK_COPYPOS, "Copy Position", JMConstants.Lucide("copy"));
		m_MarkerMenu.AddItem(MENU_MK_FOCUS, "Center Map Here", JMConstants.Lucide("locate-fixed"));

		if (m_MarkerMenuLabel != "")
			m_MarkerMenu.AddItem(MENU_MK_COPYNAME, "Copy Name", JMConstants.Lucide("copy"));

		m_MarkerMenu.SetItemEnabled(MENU_MK_TELEPORT, JMPermissions.Has(JMConstants.PERM_PLAYER_TELEPORT_POSITION));

		int mx, my;
		GetMousePos(mx, my);

		m_MarkerMenu.OpenAt(mx, my);
	}

	void OnClick_MarkerMenu(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_MarkerMenu)
			return;

		string id = m_MarkerMenu.GetLastClickedId();

		if (id == MENU_MK_TELEPORT)
		{
			if (!JMPermissions.Has(JMConstants.PERM_PLAYER_TELEPORT_POSITION))
				return;

			JMTeleportModule teleportModule = CF_Modules<JMTeleportModule>.Get();
			JMPlayerInstance self = GetPermissionsManager().GetClientPlayer();

			if (teleportModule && self)
				teleportModule.Position(m_MarkerMenuPos, { self.GetGUID() });

			return;
		}

		if (id == MENU_MK_COPYPOS)
		{
			COTFeedback.Copy("<" + m_MarkerMenuPos[0] + ", " + m_MarkerMenuPos[1] + ", " + m_MarkerMenuPos[2] + ">");
			return;
		}

		if (id == MENU_MK_FOCUS)
		{
			if (m_MarkerMenuMap)
				m_MarkerMenuMap.CenterOn(m_MarkerMenuPos);

			return;
		}

		if (id == MENU_MK_COPYNAME)
		{
			COTFeedback.Copy(m_MarkerMenuLabel);
			return;
		}
	}
}
