#ifdef DIAG
modded class PluginDiagMenu
{
	protected string m_COT_TextDiagsRootMenu         = "COT";

	protected int m_COT_DiagsRootMenuID;
	protected int m_COT_DiagsBool_SpectatorCamDbgDraw;

	override protected void RegisterModdedDiagsIDs()
	{
		super.RegisterModdedDiagsIDs();

		m_COT_DiagsRootMenuID = GetModdedDiagID();
		m_COT_DiagsBool_SpectatorCamDbgDraw = GetModdedDiagID();
	}

	override protected void RegisterModdedDiags()
	{
		super.RegisterModdedDiags();

		DiagMenu.RegisterMenu(m_COT_DiagsRootMenuID, m_COT_TextDiagsRootMenu, GetModdedRootMenu());
		{
			DiagMenu.RegisterBool(m_COT_DiagsBool_SpectatorCamDbgDraw, "", "Spectator cam dbg draw", m_COT_DiagsRootMenuID);
		}
	}
};

modded class PluginDiagMenuClient
{
	override protected void BindCallbacks()
	{
		super.BindCallbacks();

		DiagMenu.BindCallback(m_COT_DiagsBool_SpectatorCamDbgDraw, COT_SpectatorCamDbgDraw);
	}

	static void COT_SpectatorCamDbgDraw(bool state)
	{
		JMSpectatorCamera.s_DbgDraw = state;
	}
};
#endif
