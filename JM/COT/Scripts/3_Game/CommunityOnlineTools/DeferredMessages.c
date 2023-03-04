class JMDeferredMessage
{
	static ref array<ref JMDeferredMessage> QueuedMessages = new array<ref JMDeferredMessage>;

	string Title;
	string Message;

	private void JMDeferredMessage(string title, string message)
	{
		Title = title;
		Message = message;
	}

	void ~JMDeferredMessage()
	{
		CF_Log.Trace("~JMDeferredMessage");
	}

	static void Queue(string title, string message)
	{
		QueuedMessages.Insert(new JMDeferredMessage(title, message));
	}
};

