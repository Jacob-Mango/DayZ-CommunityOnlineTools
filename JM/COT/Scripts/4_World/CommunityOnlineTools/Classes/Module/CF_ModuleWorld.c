modded class CF_ModuleWorld
{
	override void OnInit()
	{
		super.OnInit();
		
		COTWebhook.RegisterWebhooks(this);
	}

	string GetWebhookTitle()
	{
		return "Unknown";
	}

	void GetSubCommands(inout array<ref JMCommand> commands)
	{
	}

	void AddSubCommand(inout array<ref JMCommand> commands, string command, string function, string permission)
	{
		commands.Insert(new JMSubCommand(this, command, function, permission));
	}

	array<string> GetCommandNames()
	{
		return new array<string>();
	}
};
