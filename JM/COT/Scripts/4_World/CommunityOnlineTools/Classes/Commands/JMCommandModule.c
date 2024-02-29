class JMCommandModule: JMModuleBase
{
	private ref map<string, ref map<string, ref JMCommand>> m_CommandMap = new map<string, ref map<string, ref JMCommand>>();

	void JMCommandModule() 
	{
	}

	override void EnableUpdate()
	{
	}

	override void OnMissionStart()
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

	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		switch (rpc_type)
		{
			case JMCommandModuleRPC.PerformCommand:
			{
				#ifdef JM_COT_DIAG_LOGGING
				auto trace = CF_Trace_1(this, "JMCommandModuleRPC.PerformCommand").Add(sender);
				#endif

				string input;
				if (!ctx.Read(input)) return;

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

						subCommand.Execute(sender, tokens);
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