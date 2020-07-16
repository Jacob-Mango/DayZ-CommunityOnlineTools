class JMWebhookDiscordMessage : JMWebhookMessage
{
    string username;
    string avatar_url;
    string content;

    ref array< ref JMWebhookDiscordEmbed > embeds;

    void JMWebhookDiscordMessage()
    {
        embeds = new array< ref JMWebhookDiscordEmbed >;
    }

    void ~JMWebhookDiscordMessage()
    {
        delete embeds;
    }

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

    ref JMWebhookDiscordEmbed CreateEmbed()
    {
        JMWebhookDiscordEmbed embed = new JMWebhookDiscordEmbed;
        embeds.Insert( embed );
        return embed;
    }

    ref JMWebhookDiscordEmbed GetEmbed( int idx = 0 )
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

    override string Prepare( ref JsonSerializer serializer )
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
    ref JMWebhookDiscordEmbedAuthor author = NULL;

    string title;
    string url;
    string description;
    int color;

    ref array< ref JMWebhookDiscordEmbedField > fields;

    ref JMWebhookDiscordEmbedThumbnail thumbnail = NULL;

    ref JMWebhookDiscordEmbedImage image = NULL;

    ref JMWebhookDiscordEmbedFooter footer = NULL;

    void JMWebhookDiscordEmbed()
    {
        fields = new array< ref JMWebhookDiscordEmbedField >;
    }

    void ~JMWebhookDiscordEmbed()
    {
        delete author;
        delete fields;
        delete thumbnail;
        delete image;
        delete footer;
    }

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
        ref JMWebhookDiscordEmbedField field = new JMWebhookDiscordEmbedField;

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