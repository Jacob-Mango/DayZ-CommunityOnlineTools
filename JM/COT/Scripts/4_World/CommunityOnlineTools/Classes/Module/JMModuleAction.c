//! One client -> server admin action, described once; JMModuleBase does Submit() / RunAction().
//! Add via a subclass plus DefineAction( Cls ) in RegisterActions() and `default: RunAction(
//! sender, rpc_type, ctx );` in OnRPC(). Needs a no-arg constructor; wire format is the order
//! of Write()/Read().
class JMModuleAction
{
	//! The RPC that carries this action from an admin's client to the server.
	int GetRPC()
	{
		return 0;
	}

	//! Node the sender must hold. An action that names none is REFUSED, not open to everyone:
	//! forgetting to gate an action must not be the way to make it public.
	string GetPermission()
	{
		return "";
	}

	//! Webhook event type. Empty sends no webhook. Must be listed by the module's GetWebhookTypes(),
	//! and is added to it automatically for a module that describes itself with JMModuleInfo.
	string GetWebhookType()
	{
		return "";
	}

	int GetWebhookColor()
	{
		return JMConstants.WEBHOOK_COLOR_INFO;
	}

	//! Client side: put the payload on the wire.
	void Write( ParamsWriteContext ctx )
	{
	}

	//! Server side: take the payload off the wire. False for a short or malformed message, which
	//! drops the action before anything is checked or applied.
	bool Read( ParamsReadContext ctx )
	{
		return true;
	}

	//! Server side, after the permission check: do the work. `sender` is NULL when the host
	//! executes its own action; `admin` may be NULL for the same reason. Return false when
	//! nothing happened - no log line and no webhook are written for it.
	bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
	{
		return true;
	}

	//! The line the log carries. Empty writes neither the log line nor the webhook.
	string Describe()
	{
		return "";
	}

	//! The line the webhook carries, when it should read differently from the log (markdown
	//! emphasis, say). Defaults to Describe().
	string DescribeWebhook()
	{
		return Describe();
	}
}
