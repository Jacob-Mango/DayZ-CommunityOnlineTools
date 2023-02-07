class JMTestingModulePlayerIdentityC : Managed
{
	PlayerIdentity identity;
};

class JMTestingModule : JMModuleBase
{
	//ref array<ref PlayerIdentity> m_Identities = new array<ref PlayerIdentity>();

	override void EnableUpdate()
	{
	}

	override void OnInvokeConnect(PlayerBase player, PlayerIdentity identity)
	{
		//m_Identities.Insert(identity);
	}

	void Command_ViewA(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		Print("Command_ViewA");

		array<PlayerIdentity> identities();
		GetGame().GetPlayerIndentities(identities);

		int index = 0;
		foreach (auto identity : identities)
		{
			string message = "";
			message += "[" + index + "]";
			message += " Name: " + identity.GetName();
			message += " FullName: " + identity.GetFullName();
			message += " Id: " + identity.GetId();
			message += " PlainId: " + identity.GetPlainId();
			message += " PlayerId: " + identity.GetPlayerId();

			Print("" + message);

			index++;
		}
	}

	void Command_ViewB(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		Print("Command_ViewB");

		array<PlayerIdentity> identities();
		GetGame().GetPlayerIndentities(identities);

		PlayerIdentity identity = null;
		for (int index = 0; index < identities.Count(); index++)
		{
			identity = identities[index];

			string message = "";
			message += "[" + index + "]";
			message += " Name: " + identity.GetName();
			message += " FullName: " + identity.GetFullName();
			message += " Id: " + identity.GetId();
			message += " PlainId: " + identity.GetPlainId();
			message += " PlayerId: " + identity.GetPlayerId();

			Print("" + message);
		}
	}

	void Command_ViewC(JMCommandParameterList params, PlayerIdentity sender, JMPlayerInstance instance)
	{
		Print("Command_ViewC");

		array<PlayerIdentity> identities();
		GetGame().GetPlayerIndentities(identities);

		JMTestingModulePlayerIdentityC identity = null;
		for (int index = 0; index < identities.Count(); index++)
		{
			identity = new JMTestingModulePlayerIdentityC();
			identity.identity = identities[index];

			string message = "";
			message += "[" + index + "]";
			message += " Name: " + identity.identity.GetName();
			message += " FullName: " + identity.identity.GetFullName();
			message += " Id: " + identity.identity.GetId();
			message += " PlainId: " + identity.identity.GetPlainId();
			message += " PlayerId: " + identity.identity.GetPlayerId();

			Print("" + message);
		}
	}

	override void GetSubCommands(inout array<ref JMCommand> commands)
	{
		AddSubCommand(commands, "viewa", "Command_ViewA", "Admin");
		AddSubCommand(commands, "viewb", "Command_ViewB", "Admin");
		AddSubCommand(commands, "viewc", "Command_ViewC", "Admin");
	}

	override array<string> GetCommandNames()
	{
		auto names = new array<string>();
		names.Insert("test");
		return names;
	}
};