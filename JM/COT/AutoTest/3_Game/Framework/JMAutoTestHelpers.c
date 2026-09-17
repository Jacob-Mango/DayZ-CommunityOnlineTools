#ifdef JM_COT_AUTOTEST

//! Assertion library for the JM COT autotest boot pass. Targets deterministic,
//! world-independent logic (permission math, RPC throttling, constant/config
//! builders) - things with a correct/incorrect answer that don't need a live
//! world to check. Ported from EgoLand's AutoTestHelpers.c.
//!
//! Two counter scopes: g_JMAutoTestPassCount/FailCount accumulate across the
//! WHOLE run and are what the runner reads for the final verdict. Suites must
//! never reset them - the runner takes before/after snapshots per suite
//! instead. Every failed assertion prints expected-vs-actual and raises an
//! engine Error() so a failed run cannot hide in the log.

static int g_JMAutoTestPassCount = 0;
static int g_JMAutoTestFailCount = 0;
static int g_JMAutoTestGroupCount = 0;

static void JMAutoTest_Pass(string label)
{
	g_JMAutoTestPassCount++;
	PrintFormat("  [PASS] %1", label);
}

static void JMAutoTest_Fail(string label, string detail)
{
	g_JMAutoTestFailCount++;
	PrintFormat("  [FAIL] %1  --  %2", label, detail);
	Error("[JM_COT_AUTOTEST][FAIL] " + label + " -- " + detail);
}

static void JMAutoTest_Assert(string label, bool condition)
{
	if (condition)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected TRUE, got FALSE");
}

static void JMAutoTest_AssertFalse(string label, bool condition)
{
	if (!condition)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected FALSE, got TRUE");
}

static void JMAutoTest_AssertEqualInt(string label, int got, int expected)
{
	if (got == expected)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected " + expected + ", got " + got);
}

static void JMAutoTest_AssertEqualStr(string label, string got, string expected)
{
	if (got == expected)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected '" + expected + "', got '" + got + "'");
}

//! Inclusive integer range check: low <= got <= high
static void JMAutoTest_AssertInRange(string label, int got, int low, int high)
{
	if (got >= low && got <= high)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected [" + low + ".." + high + "], got " + got);
}

//! Float near-equality within an absolute tolerance.
static void JMAutoTest_AssertFloatNear(string label, float got, float expected, float tol)
{
	float d = got - expected;
	if (d < 0) d = -d;
	if (d <= tol)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected " + expected + " +/-" + tol + ", got " + got + " (off " + d + ")");
}

//! Angle near-equality (degrees), accounting for wraparound: 180 == -180,
//! 270 == -90, 0 == 360. Compares the shortest signed delta against tolerance.
static void JMAutoTest_AssertAngleNear(string label, float got, float expected, float tol)
{
	float d = got - expected;
	while (d > 180.0) d = d - 360.0;
	while (d < -180.0) d = d + 360.0;
	if (d < 0) d = -d;
	if (d <= tol)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected " + expected + " deg +/-" + tol + ", got " + got + " deg (off " + d + " deg)");
}

//! Vector near-equality: Euclidean distance within tolerance (metres).
static void JMAutoTest_AssertVectorNear(string label, vector got, vector expected, float tol)
{
	float d = vector.Distance(got, expected);
	if (d <= tol)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected " + expected.ToString() + " +/-" + tol + "m, got " + got.ToString() + " (off " + d + "m)");
}

static void JMAutoTest_AssertNotNull(string label, Class obj)
{
	if (obj != null)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected non-null, got null");
}

static void JMAutoTest_AssertNull(string label, Class obj)
{
	if (obj == null)
		JMAutoTest_Pass(label);
	else
		JMAutoTest_Fail(label, "expected null, got non-null");
}

//! Cosmetic section header inside a suite - does not gate anything.
static void JMAutoTest_Group(string name)
{
	g_JMAutoTestGroupCount++;
	PrintFormat("");
	PrintFormat("  [GROUP %1] %2", g_JMAutoTestGroupCount, name);
}

#endif
