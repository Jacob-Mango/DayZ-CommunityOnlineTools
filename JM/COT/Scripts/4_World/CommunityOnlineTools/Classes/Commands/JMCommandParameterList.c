class JMCommandParameterList
{
	ref array<string> m_Arguments;
	int m_Index;
	
	void JMCommandParameterList(array<string> arguments)
	{
		m_Arguments = new array<string>();
		m_Arguments.Copy(arguments);

		m_Index = -1;
	}
	
	string Current()
	{
		return m_Arguments[m_Index];
	}

	bool HasNext()
	{
		return m_Arguments.Count() > (m_Index + 1);
	}
	
	bool Next()
	{
		m_Index++;
		return m_Index < m_Arguments.Count();
	}
	
	bool Next(out int param)
	{
		if (!Next()) return false;
		
		param = Current().ToInt();
		return true;
	}
	
	bool Next(out float param)
	{
		if (!Next()) return false;
		
		param = Current().ToFloat();
		return true;
	}
	
	bool Next(out string param)
	{
		if (!Next()) return false;
		
		param = Current();
		return true;
	}
	
	bool Next(out JMPlayerInstance param)
	{
		if (!Next()) return false;
		
		param = GetPermissionsManager().GetPlayer(Current());
		return param != null;
	}
	
	bool Next(out PlayerBase param)
	{
		if (!Next()) return false;
		
		JMPlayerInstance instance = GetPermissionsManager().GetPlayer(Current());
		if (instance == null) return false;
		param = instance.PlayerObject;
		return param != null;
	}
};
