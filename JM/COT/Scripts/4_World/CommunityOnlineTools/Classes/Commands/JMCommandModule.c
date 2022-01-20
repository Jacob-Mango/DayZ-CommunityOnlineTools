enum JMCommandModuleRPC
{
	INVALID = 10420,
	PerformCommand,
	COUNT
};

[CF_RegisterModule(JMCommandModule)]
class JMCommandModule: CF_ModuleWorld
{
	private ref map<string, ref map<string, ref JMCommand>> m_CommandMap = new map<string, ref map<string, ref JMCommand>>();
	
	override void OnInit()
	{
		super.OnInit();
		
		EnableMissionStart();
	}

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		JMCommandConstructor.Generate(m_CommandMap);
	}
	
	override int GetRPCMin()
	{
		return JMCommandModuleRPC.INVALID;
	}
	
	override int GetRPCMax()
	{
		return JMCommandModuleRPC.COUNT;
	}

	override void OnRPC(Class sender, CF_EventArgs args)
	{
		auto rpc = CF_EventRPCArgs.Cast(args);
		switch (rpc.ID)
		{
			case JMCommandModuleRPC.PerformCommand:
			{
				string input;
				if (!rpc.Context.Read(input)) return;

				array<string> tokens = new array<string>();
				input.Split(" ", tokens);

				string strCommand = tokens[0];
				strCommand.ToLower();

				string strSubCommand = tokens[1];
				strSubCommand.ToLower();

				auto subCommands = m_CommandMap[strCommand];
				if (subCommands)
				{
					auto subCommand = subCommands[strSubCommand];
					if (subCommand)
					{
						tokens.RemoveOrdered(0);
						tokens.RemoveOrdered(0);

						subCommand.Execute(rpc.Sender, tokens);
					}
				}
			}
		}
	}

	void PerformCommand(string input)
	{
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(input);
		rpc.Send(null, JMCommandModuleRPC.PerformCommand, true, null);
	}
};