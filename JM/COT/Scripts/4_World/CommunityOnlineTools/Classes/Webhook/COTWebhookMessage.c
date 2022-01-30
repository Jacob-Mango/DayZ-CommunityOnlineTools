class COTWebhookMessage : Managed
{
	COTWebhook _webhook;

	void COTWebhookMessage(COTWebhook webhook = null)
	{
		_webhook = webhook;
	}

	void Send()
	{
		_webhook.Send(this);
	}

	string Prepare(JsonSerializer serializer)
	{
		return "{}";
	}

	string GetType()
	{
		return "Base";
	}
};
