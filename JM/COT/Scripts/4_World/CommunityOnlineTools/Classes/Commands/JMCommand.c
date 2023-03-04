class JMCommand
{
	protected string m_Command;

	protected Class m_Instance;
	protected string m_Function;

	protected string m_Permission;

	void JMCommand(Class instance, string command, string function, string permission)
	{
		m_Instance = instance;
		m_Command = command;
		m_Function = function;
		m_Permission = permission;
	}

	void Execute(PlayerIdentity sender, array<string> arguments)
	{
		JMPlayerInstance instance;
		if (!GetPermissionsManager().HasPermission(m_Permission, sender, instance)) return;

		g_Script.CallFunctionParams(m_Instance, m_Function, null, new Param3<ref JMCommandParameterList, PlayerIdentity, JMPlayerInstance>(new JMCommandParameterList(arguments), sender, instance));
	}

	string GetCommand()
	{
		return m_Command;
	}

	Class GetInstance()
	{
		return m_Instance;
	}

	string GetFunction()
	{
		return m_Function;
	}
};

class JMSubCommand: JMCommand
{
};
