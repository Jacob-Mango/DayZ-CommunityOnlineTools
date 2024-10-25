modded class ImprovisedExplosive
{
	override void COT_OnDebugSpawn(PlayerBase player)
	{
		GetInventory().CreateAttachment("Plastic_Explosive");
		GetInventory().CreateAttachment("Plastic_Explosive");

		UnlockTriggerSlots();

		Object triggerObj = player.GetHumanInventory().CreateInHands("RemoteDetonatorTrigger");
		if (!triggerObj)
			triggerObj = GetGame().CreateObject("RemoteDetonatorTrigger", player.GetPosition());

		RemoteDetonatorTrigger trigger = RemoteDetonatorTrigger.Cast(triggerObj);
		RemoteDetonatorReceiver receiver = RemoteDetonatorReceiver.Cast(GetInventory().CreateAttachment("RemoteDetonatorReceiver"));

		MiscGameplayFunctions.TransferItemProperties(trigger, receiver);

		receiver.LockToParent();
		SetTakeable(false);

		PairWithDevice(trigger);
		Arm();
	}
};
