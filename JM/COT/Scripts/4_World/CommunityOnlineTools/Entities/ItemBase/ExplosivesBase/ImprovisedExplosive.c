modded class ImprovisedExplosive
{
	override void COT_OnDebugSpawn()
	{
		GetInventory().CreateAttachment("Plastic_Explosive");
		GetInventory().CreateAttachment("Plastic_Explosive");
		UnlockTriggerSlots();
		RemoteDetonatorTrigger trigger = RemoteDetonatorTrigger.Cast(GetGame().CreateObject("RemoteDetonatorTrigger", GetPosition()));
		RemoteDetonatorReceiver receiver = RemoteDetonatorReceiver.Cast(GetInventory().CreateAttachment("RemoteDetonatorReceiver"));
		MiscGameplayFunctions.TransferItemProperties(trigger, receiver);
		receiver.LockToParent();
		SetTakeable(false);
		PairWithDevice(trigger);
		Arm();
	}
}
