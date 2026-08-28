#ifdef JM_COT
class ExpansionConfigReloadCOTForm: JMFormBase
{
	protected Widget m_ActionsWrapper;
	protected ExpansionConfigReloadCOTModule m_Module;

	#ifdef COT_BUGFIX_REF
	protected override bool SetModule(JMRenderableModuleBase mdl)
	#else
	protected override bool SetModule(ref JMRenderableModuleBase mdl)
	#endif
	{
		return Class.CastTo(m_Module, mdl);
	}

	override void OnInit()
	{
		m_ActionsWrapper = layoutRoot.FindAnyWidget("actions_wrapper");
		if (!m_ActionsWrapper || !m_Module)
		{
			return;
		}

		UIActionManager.CreateText(m_ActionsWrapper, Widget.TranslateString("#STR_ECR_Menu_Description"));

		#ifdef EXPANSIONMODMARKET
		if (ExpansionConfigReloadFeatures.HasMarket())
		{
			UIActionManager.CreateButton(m_ActionsWrapper, Widget.TranslateString("#STR_ECR_Button_Market"), this, "Click_ReloadMarket");
		}
		#endif

		#ifdef EXPANSIONMODQUESTS
		if (ExpansionConfigReloadFeatures.HasQuests())
		{
			UIActionManager.CreateButton(m_ActionsWrapper, Widget.TranslateString("#STR_ECR_Button_Quests"), this, "Click_ReloadQuests");
		}
		#endif

		#ifdef EXPANSIONMODMISSIONS
		if (ExpansionConfigReloadFeatures.HasAirdrop())
		{
			UIActionManager.CreateButton(m_ActionsWrapper, Widget.TranslateString("#STR_ECR_Button_Airdrop"), this, "Click_ReloadAirdrop");
		}
		#endif

		if (ExpansionConfigReloadFeatures.HasLoadouts())
		{
			UIActionManager.CreateButton(m_ActionsWrapper, Widget.TranslateString("#STR_ECR_Button_Loadouts"), this, "Click_ReloadLoadouts");
		}
	}

	#ifdef EXPANSIONMODMARKET
	void Click_ReloadMarket(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module)
		{
			return;
		}

		m_Module.RequestReloadMarket();
	}
	#endif

	#ifdef EXPANSIONMODQUESTS
	void Click_ReloadQuests(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module)
		{
			return;
		}

		m_Module.RequestReloadQuests();
	}
	#endif

	#ifdef EXPANSIONMODMISSIONS
	void Click_ReloadAirdrop(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module)
		{
			return;
		}

		m_Module.RequestReloadAirdrop();
	}
	#endif

	void Click_ReloadLoadouts(UIEvent eid, UIActionBase action)
	{
		if (eid != UIEvent.CLICK || !m_Module)
		{
			return;
		}

		m_Module.RequestReloadLoadouts();
	}
}
#endif
