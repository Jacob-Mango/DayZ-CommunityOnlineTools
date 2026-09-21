modded class ActionTargetsCursor
{
	override protected string GetActionDesc(ActionBase action)
	{
		string desc = super.GetActionDesc(action);

		AnimatedActionBase animatedAction;
		if (Class.CastTo(animatedAction, action) && animatedAction.HasTarget())
		{
			typename callbackClass = animatedAction.COT_GetCallbackClassTypename();
			if (callbackClass.IsInherited(ActionContinuousBaseCB))
			{
				if (GetCommunityOnlineToolsBase().IsActive() && JMPermissions.Has(JMConstants.PERM_ACTIONS_QUICKACTIONS))
					desc = "[COT QuickAction] " + desc;
			}
		}

		return desc;
	}
}
