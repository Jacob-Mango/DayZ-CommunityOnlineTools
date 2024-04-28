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
				if (GetCommunityOnlineToolsBase().IsActive() && GetPermissionsManager().HasPermission("Actions.QuickActions"))
					desc = "[COT QuickAction] " + desc;
			}
		}

		return desc;
	}
}
