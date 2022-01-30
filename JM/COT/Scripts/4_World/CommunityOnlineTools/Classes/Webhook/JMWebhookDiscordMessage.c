class JMWebhookDiscordMessage : COTWebhookMessage
{
	string username;
	string avatar_url;
	string content;

	autoptr array< ref JMWebhookDiscordEmbed > embeds = new array< ref JMWebhookDiscordEmbed >;

	void SetHeader( string username, string avatar_url, string content )
	{
		this.username = username;
		this.avatar_url = avatar_url;
		this.content = content;
	}

	void AddEmbed( notnull ref JMWebhookDiscordEmbed obj )
	{
		embeds.Insert( obj );
	}

	JMWebhookDiscordEmbed CreateEmbed()
	{
		JMWebhookDiscordEmbed embed = new JMWebhookDiscordEmbed;
		embeds.Insert( embed );
		return embed;
	}

	JMWebhookDiscordEmbed GetEmbed( int idx = 0 )
	{
		if ( embeds.Count() <= idx )
			return embeds[idx];
		
		for ( int i = embeds.Count(); i <= idx; ++i )
		{
			JMWebhookDiscordEmbed embed = new JMWebhookDiscordEmbed;
			embeds.Insert( embed );
		}
		
		return embeds[idx];
	}

	override string Prepare( JsonSerializer serializer )
	{
		string result = "";
		serializer.WriteToString( embeds, false, result );

		string prepend = "{";
		if (username != "") prepend += "\"username\": " + username + ",";
		if (avatar_url != "") prepend += "\"avatar_url\": " + avatar_url + ",";
		if (content != "") prepend += "\"content\": " + content + ",";
		return prepend + "\"embeds\": " + result + "}";
	}

	override string GetType()
	{
		return "Discord";
	}
};

class JMWebhookDiscordEmbedAuthor : Managed
{
	string name;
	string url;
	string icon_url;
};

class JMWebhookDiscordEmbedField : Managed
{
	string name;
	string value;
	bool inline;
};

class JMWebhookDiscordEmbedThumbnail : Managed
{
	string url;
};

class JMWebhookDiscordEmbedImage : Managed
{
	string url;
};

class JMWebhookDiscordEmbedFooter : Managed
{
	string text;
	string icon_url;
};

class JMWebhookDiscordEmbed : Managed
{
	autoptr JMWebhookDiscordEmbedAuthor author = NULL;

	string title;
	string url;
	string description;
	int color;

	autoptr array< ref JMWebhookDiscordEmbedField > fields = new array< ref JMWebhookDiscordEmbedField >;
	autoptr JMWebhookDiscordEmbedThumbnail thumbnail = NULL;
	autoptr JMWebhookDiscordEmbedImage image = NULL;
	autoptr JMWebhookDiscordEmbedFooter footer = NULL;

	void SetTitle( string txt )
	{
		title = txt;
	}

	void SetURL( string txt )
	{
		url = txt;
	}

	void SetDescription( string txt )
	{
		description = txt;
	}

	void SetColor( int txt )
	{
		color = txt;
	}

	void SetAuthor( string name, string url = "", string icon_url = "" )
	{
		if ( !author )
			author = new JMWebhookDiscordEmbedAuthor;

		author.name = name;
		author.url = url;
		author.icon_url = icon_url;
	}

	void AddField( string name, string value, bool inline = false )
	{
		JMWebhookDiscordEmbedField field = new JMWebhookDiscordEmbedField;

		field.name = name;
		field.value = value;
		field.inline = inline;

		fields.Insert( field );
	}

	void SetThumbnail( string url )
	{
		if ( !thumbnail )
			thumbnail = new JMWebhookDiscordEmbedThumbnail;

		thumbnail.url = url;
	}

	void SetImage( string url )
	{
		if ( !image )
			image = new JMWebhookDiscordEmbedImage;

		image.url = url;
	}

	void SetFooter( string text, string icon_url = "" )
	{
		if ( !footer )
			footer = new JMWebhookDiscordEmbedFooter;

		footer.text = text;
		footer.icon_url = icon_url;
	}
};
