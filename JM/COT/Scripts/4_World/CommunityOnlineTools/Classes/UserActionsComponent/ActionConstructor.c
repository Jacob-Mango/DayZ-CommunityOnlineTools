modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(COT_QuickActionActivateTrap);
		actions.Insert(COT_ActionTriggerBearTrap);
	}
}
