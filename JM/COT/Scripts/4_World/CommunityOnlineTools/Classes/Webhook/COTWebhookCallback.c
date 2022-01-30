class COTWebhookCallback : RestCallback
{
	override void OnSuccess(string data, int dataSize)
	{
	}

	override void OnError(int errorCode)
	{
		if (errorCode < ERestResultState.EREST_ERROR)
			return;

		CF_Log.Error("OnError - " + errorCode);
	}

	override void OnTimeout()
	{
	}
};
