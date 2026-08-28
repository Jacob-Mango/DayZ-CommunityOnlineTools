modded class JMObjectSpawnerForm
{
	string m_ZenDeleteAllEntityType;
	UIActionButton m_ZenShowOnMapButton;
	UIActionButton m_ZenDeleteAllButton;
	
	override void OnInit()
	{
		super.OnInit();
		
		if (m_ItemDataList && m_ItemDataList.GetLayoutRoot())
		{
			m_ItemDataList.GetLayoutRoot().Show(false);
		}

		// Panel that contains the search box + "X" button
		Widget actions = m_SearchBox.GetLayoutRoot().GetParent();

		if (!m_ZenShowOnMapButton && actions)
		{
			m_ZenShowOnMapButton = UIActionManager.CreateButton(actions, "#STR_ZenCOT_Map", this, "OnClick_ZenShowOnMap");
			m_ZenShowOnMapButton.SetWidth(0.10);
			m_ZenShowOnMapButton.SetPosition(0.44);

			m_ZenDeleteAllButton = UIActionManager.CreateButton(actions, "#STR_ZenCOT_MapClear", this, "OnClick_ZenDeleteAllButton");
			m_ZenDeleteAllButton.SetWidth(0.15);
			m_ZenDeleteAllButton.SetPosition(0.55);
			m_ZenDeleteAllButton.SetColor(COLOR_RED);
		}
	}
	
	void OnClick_ZenDeleteAllButton(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK) 
			return;

		JMMapModule mapMod;
		if (Class.CastTo(mapMod, GetModuleManager().GetModule(JMMapModule)))
		{
			mapMod.SetTempItemMarks(new array<ref JMMapTempMark>, "Markers");
		}
	}

	void OnClick_ZenShowOnMap(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK) 
			return;

		string selected;
		if (m_ClassList && m_ClassList.GetSelectedRow() != -1)
			m_ClassList.GetItemText(m_ClassList.GetSelectedRow(), 0, selected);
		else 
		if (m_SearchBox)
			selected = m_SearchBox.GetText();

		if (selected != "" && m_Module)
		{
			GetRPCManager().SendRPC("ZenCOT_RPC", "RPC_ZenShowOnMapRequest", new Param1<string>(selected), true);
		}
	}

	private void ZenDeleteAll_Yes(JMConfirmation confirmation)
	{
		GetRPCManager().SendRPC("ZenCOT_RPC", "RPC_ZenDeleteAllOnMap", new Param1<string>(m_ZenDeleteAllEntityType), true, null);
		m_ZenDeleteAllEntityType = "";

		if (m_Module && m_Module.m_AutoShow)
			m_Module.Hide();
	}

	private void ZenDeleteAll_No(JMConfirmation confirmation)
	{
		m_ZenDeleteAllEntityType = "";

		if (m_Module && m_Module.m_AutoShow)
			m_Module.Hide();
	}
}