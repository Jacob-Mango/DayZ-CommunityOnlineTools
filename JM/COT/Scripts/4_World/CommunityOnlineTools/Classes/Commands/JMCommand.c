enum JMCommandParameterType
{
	STRING,
	FLOAT,
	VECTOR,
	INT,
	PLAYER,
	OBJECT //! Don't think 
};

class JMCommandParameter
{
	JMCommandParameterType m_Type;
	string m_Name;

	//string ToString()
	//{
	//	string type = "UNKNOWN";
	//	return "[" + m_Name + ":" + type + "]";
	//}
};

class JMCommand
{
	private string m_Command;

	private Class m_Instance;
	private string m_Function;

	private ref array< JMCommandParameter > m_Parameters;
	private ref Param m_PassThroughParameters;

	void JMCommand( string command, string function )
	{
		m_Command = command;
		m_Function = function;
		m_Parameters = new array< JMCommandParameter >();
	}

	void ~JMCommand()
	{
		delete m_PassThroughParameters;
	}

	void AddParameter( JMCommandParameterType type, string name )
	{
		m_Parameters.Insert( new JMCommandParameter() );
	}

	/**
	 * Called once the command is finished initializing
	 */
	void Finish()
	{
		//m_Parameters
		//m_PassThroughParameters = 
	}

	void Error()
	{
		
	}

	void Execute( array< string > arguments )
	{
		//g_Script.Call( m_Instance, m_Function );
	}
};