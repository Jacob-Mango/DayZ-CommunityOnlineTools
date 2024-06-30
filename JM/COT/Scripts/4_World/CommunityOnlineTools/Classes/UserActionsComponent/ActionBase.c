#ifndef SERVER
modded class ActionBase
{
	override bool Can(PlayerBase player, ActionTarget target, ItemBase item, int condition_mask)
	{
		if (!super.Can(player, target, item, condition_mask))
			return false;

		if (GetCommunityOnlineToolsBase().IsOpen() || GetCOTWindowManager().HasAnyActive() || GetCOTWindowManager().PendingDeletionCount() > 0)
			return false;

		return true;
	}
};

#endif
