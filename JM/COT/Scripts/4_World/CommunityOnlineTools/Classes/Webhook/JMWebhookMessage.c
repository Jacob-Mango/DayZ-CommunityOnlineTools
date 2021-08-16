class JMWebhookMessage : Managed
{
	string Prepare( JsonSerializer serializer )
	{
		return "{}";
	}

	string GetType()
	{
		return "Base";
	}
};