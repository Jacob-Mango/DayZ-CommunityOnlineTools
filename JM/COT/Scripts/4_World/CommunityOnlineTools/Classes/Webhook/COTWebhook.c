class COTWebhook
{
	static autoptr map<string, ref COTWebhook> s_All = new map<string, ref COTWebhook>();

	string m_Type;
	string m_Title;

	COTWebhookModule m_Module;

	private void COTWebhook(string type)
	{
		m_Type = type;

		CF_Modules<COTWebhookModule>.Get(m_Module);
	}

	/*private*/ void ~COTWebhook()
	{
	}

	void Send(string message)
	{
		if (!m_Module || IsMissionOffline())
			return;

		auto msg = Create();
		msg.GetEmbed().AddField(m_Title, message, false);
		m_Module.Post(m_Type, msg);
	}

	void Send(JMPlayerInstance player, string message)
	{
		if (!m_Module || !player || IsMissionOffline())
			return;

		auto msg = Create(player, "Admin Account: ");
		msg.GetEmbed().AddField(m_Title, message, false);
		m_Module.Post(m_Type, msg);
	}

	void Send(COTWebhookMessage message)
	{
		m_Module.Post(m_Type, message);
	}

	JMWebhookDiscordMessage Create()
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( 16766720 );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );
		
		if ( m_Module.m_ServerHostName != "" )
			embed.AddField("Server: ", m_Module.m_ServerHostName, false );

		return message;
	}

	JMWebhookDiscordMessage Create(JMPlayerInstance player, string playerTitle)
	{
		JMWebhookDiscordMessage message = new JMWebhookDiscordMessage;
		auto embed = message.CreateEmbed();
		embed.SetColor( 16766720 );

		embed.SetAuthor( "Community Online Tools", "https://steamcommunity.com/sharedfiles/filedetails/?id=1564026768", "https://steamuserimages-a.akamaihd.net/ugc/960854969917124348/1A32B80495D9F205E4D91C61AE309D19A44A8B92/" );

		if ( m_Module.m_ServerHostName != "" )
			embed.AddField("Server: ", m_Module.m_ServerHostName, true );

		embed.AddField(playerTitle, player.FormatSteamWebhook(), m_Module.m_ServerHostName != "" );

		return message;
	}

	static void SetWebhook(Class instance, string varName)
	{
		string webHookName = varName;

		if (webHookName.IndexOf("_") == 0)
		{
			return;
		}

		if (webHookName.IndexOf("m_") == 0)
		{
			webHookName = webHookName.Substring(2, webHookName.Length() - 2);
		}

		if (webHookName.IndexOf("WH") == 0)
		{
			webHookName = webHookName.Substring(2, webHookName.Length() - 2);
		}

		if (webHookName.IndexOf("_") == 0)
		{
			webHookName = webHookName.Substring(1, webHookName.Length() - 1);
		}

		string name = instance.ClassName() + "_" + webHookName;

		COTWebhook webhook;
		if (!s_All.Find(name, webhook))
		{
			webhook = new COTWebhook(name);
			s_All[name] = webhook;

			g_Script.CallFunction(instance, "GetWebhookTitle", webhook.m_Title, null);
		}

		EnScript.SetClassVar(instance, varName, 0, webhook);
	}

	static void RegisterWebhooks(Class instance)
	{
		typename type = instance.Type();
		int count = type.GetVariableCount();
		for (int index = 0; index < count; index++)
		{
			typename varType = type.GetVariableType(index);
			if (!varType.IsInherited(COTWebhook))
				continue;

			string varName = type.GetVariableName(index);

			SetWebhook(instance, varName);
		}
	}
};
