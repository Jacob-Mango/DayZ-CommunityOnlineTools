class JMCommandConstructor : Managed
{
	static void Generate(inout map<string, ref map<string, ref JMCommand>> commands)
	{
		int count = CF_ModuleCoreManager.Count();
		for (int i = 0; i < count; i++)
		{
			CF_ModuleWorld module;
			if (!Class.CastTo(module, CF_ModuleCoreManager.Get(i))) continue;
			
			array<string> commandNames = module.GetCommandNames();
			if (commandNames.Count() == 0) continue;

			map<string, ref JMCommand> subCommands = new map<string, ref JMCommand>();
			for (int j = 0; j < commandNames.Count(); j++) commands.Insert(commandNames[j], subCommands);

			array<ref JMCommand> subCommandsArr = new array<ref JMCommand>;
			module.GetSubCommands(subCommandsArr);

			for (int k = 0; k < subCommandsArr.Count(); k++)
			{
				subCommands.Insert(subCommandsArr[k].GetCommand(), subCommandsArr[k]);
			}
		}
	}
};