class JMWebhookMessage : Managed
{
	string GetType()
	{
		return "Base";
	}

	string Prepare( JsonSerializer serializer )
	{
		return "{}";
	}
}