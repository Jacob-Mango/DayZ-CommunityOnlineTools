class JMWebhookMessage : Managed
{
    string Prepare( ref JsonSerializer serializer )
    {
        return "{}";
    }

    string GetType()
    {
        return "Base";
    }
};