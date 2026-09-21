//! One client -> server admin action, described once.
//!
//! An action owns everything that differs between actions - the RPC it travels on, the
//! permission that gates it, its payload, what it does, and how it reads in the log and the
//! webhook. Everything that is the SAME lives once in JMModuleBase: Submit() sends it (or runs it
//! when this process is the host), RunAction() receives, reads, permission-gates and applies it,
//! and the log line and webhook are written afterwards. Adding an action is therefore one
//! subclass and one line in the module's RegisterActions() - no Send / Exec / RPC trio.
//!
//!   class JMMyModuleDelete: JMModuleAction
//!   {
//!       string Name;
//!
//!       override int GetRPC() { return JMMyModuleRPC.Delete; }
//!       override string GetPermission() { return JMConstants.PERM_MYMODULE_DELETE; }
//!       override string GetWebhookType() { return "Delete"; }
//!
//!       override void Write( ParamsWriteContext ctx ) { ctx.Write( Name ); }
//!       override bool Read( ParamsReadContext ctx ) { return ctx.Read( Name ); }
//!
//!       override bool Apply( JMModuleBase module, PlayerIdentity sender, JMPlayerInstance admin )
//!       {
//!           JMMyModule mine = JMMyModule.Cast( module );
//!           return mine && mine.DeleteByName( Name );
//!       }
//!
//!       override string Describe() { return "Deleted " + Name; }
//!   }
//!
//!   //! In the module:
//!   override void RegisterActions()
//!   {
//!       super.RegisterActions();
//!       DefineAction( JMMyModuleDelete );
//!   }
//!
//!   //! Where the OnRPC() switch has no case for the id:
//!   default:
//!       RunAction( sender, rpc_type, ctx );
//!       break;
//!
//!   //! From the client (or the host):
//!   JMMyModuleDelete action = new JMMyModuleDelete();
//!   action.Name = name;
//!   Submit( action );
//!
//! An action needs a constructor that takes no arguments: the server builds one to read the
//! payload into. Its wire format is whatever Write() puts down and Read() takes back, in the same
//! order, so a migrated action keeps the bytes the hand-written trio used.
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
