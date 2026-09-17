#ifdef JM_COT_AUTOTEST

//! Reusable fixtures for RPC-handler testing.
//!
//! JMAutoTest_EmptyReadContext() simulates a truncated/malformed payload - every
//! ctx.Read(x) inside the handler under test returns false immediately. Since
//! every handler examined in this codebase reads its payload before touching
//! senderRPC, an empty context also keeps a NULL-sender call safe: ctx.Read
//! fails and the handler returns before ever reaching an identity dereference.
//!
//! Enforce Script has no try/catch. If a handler call DOES silently abort the
//! whole script stack (a genuine VM exception, not a normal Fail), nothing
//! below it in the calling function runs - including a JMAutoTest_Pass line
//! that would otherwise prove the call returned. The canary counter below
//! makes that observable: call JMAutoTest_ExpectCanary() once before a risky
//! call, JMAutoTest_Canary(label) as the very next line after it, and
//! JMAutoTest_CanaryCheck(suiteName) once at the end of the block - a mismatch
//! between expected and fired counts means something silently aborted mid-run.

static ParamsReadContext JMAutoTest_EmptyReadContext()
{
	ScriptReadWriteContext rwctx = new ScriptReadWriteContext();
	return rwctx.GetReadContext();
}

//! A small number of handlers whose FIRST ctx.Read call is a `string` or
//! `array<string>` threw a VM exception decoding it. Root cause found this
//! session: it was never about the payload being empty - it was that the
//! write went through a chained `rwctx.GetWriteContext().Write(x)` call
//! without holding onto the ParamsWriteContext it returns. GetWriteContext()
//! hands back its own handle each call, so a write made through a throwaway
//! chained instance never lands in rwctx's buffer - the read context that
//! follows is empty regardless of what was "written", non-deterministically,
//! which is exactly the undefined-looking crash this fixture used to explain
//! away as an empty-buffer decode quirk. JMAutoTest_ReadContextForSetStat
//! never had this bug because it stores the ParamsWriteContext in a local
//! (`wctx`) and writes through that. Fixed here the same way.
static ParamsReadContext JMAutoTest_ReadContextWithOneString(string s)
{
	ScriptReadWriteContext rwctx = new ScriptReadWriteContext();
	ParamsWriteContext wctx = rwctx.GetWriteContext();
	wctx.Write(s);
	return rwctx.GetReadContext();
}

//! Same fix as JMAutoTest_ReadContextWithOneString, for handlers whose first
//! read is an array<string> (e.g. JMPlayerModule.RPC_SetRoles) rather than a
//! bare string.
static ParamsReadContext JMAutoTest_ReadContextWithOneStringArray(array<string> arr)
{
	ScriptReadWriteContext rwctx = new ScriptReadWriteContext();
	ParamsWriteContext wctx = rwctx.GetWriteContext();
	wctx.Write(arr);
	return rwctx.GetReadContext();
}

//! Fixture matching JMPlayerModule.RPC_SetStat's exact read sequence
//! (int typeInt, float value, array<string> guids) - same reasoning as
//! JMAutoTest_ReadContextWithOneString: give it real, validly-typed values
//! instead of leaning on empty-buffer default-fill behaviour, which this
//! session found to be inconsistent between runs for this handler.
static ParamsReadContext JMAutoTest_ReadContextForSetStat()
{
	ScriptReadWriteContext rwctx = new ScriptReadWriteContext();
	ParamsWriteContext wctx = rwctx.GetWriteContext();
	wctx.Write(0);
	wctx.Write(0.0);
	array<string> guids = new array<string>();
	wctx.Write(guids);
	return rwctx.GetReadContext();
}

static int g_JMAutoTestCanaryExpected = 0;
static int g_JMAutoTestCanaryFired = 0;

static void JMAutoTest_ExpectCanary()
{
	g_JMAutoTestCanaryExpected++;
}

static void JMAutoTest_Canary(string label)
{
	g_JMAutoTestCanaryFired++;
	JMAutoTest_Pass("[canary] " + label + " did not halt the suite");
}

//! Call once per suite, after every risky call block. Resets the counters for
//! whichever suite runs next.
static void JMAutoTest_CanaryCheck(string suiteName)
{
	JMAutoTest_AssertEqualInt("[canary] " + suiteName + " - all expected canaries fired (0 silent aborts)", g_JMAutoTestCanaryFired, g_JMAutoTestCanaryExpected);

	g_JMAutoTestCanaryExpected = 0;
	g_JMAutoTestCanaryFired = 0;
}

#endif
