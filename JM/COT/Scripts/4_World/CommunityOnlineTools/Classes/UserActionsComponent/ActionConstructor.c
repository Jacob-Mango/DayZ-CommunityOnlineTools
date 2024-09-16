modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		// Traps hackfix
		actions.Insert(COT_QuickActionActivateTrap);
		actions.Insert(COT_ActionTriggerBearTrap);

		// Basebuilding hackfix
		actions.Insert(COT_QuickActionBuild);
		actions.Insert(COT_QuickActionDismantle);
	}
};
