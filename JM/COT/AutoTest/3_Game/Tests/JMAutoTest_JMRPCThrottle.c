#ifdef JM_COT_AUTOTEST

//! Target: JMRPCThrottle.Allow / Clear
//! (Scripts/3_Game/CommunityOnlineTools/JMRPCThrottle.c)
//!
//! s_NextAllowed is process-global static state that persists for the life
//! of the server process - it is NOT reset between these scenarios, so every
//! scenario below uses its own unique guid/bucket pair. Reusing a key across
//! two scenarios would make the second scenario's result depend on execution
//! order instead of on the logic under test.

static void JMAutoTest_Throttle_EmptyGuidAlwaysAllowed()
{
	JMAutoTest_Group("Empty guid (server/offline-host) is never throttled");

	JMAutoTest_Assert("first call with empty guid allowed", JMRPCThrottle.Allow("", "jmautotest_bucket_a", 5000));
	JMAutoTest_Assert("immediate repeat with empty guid still allowed", JMRPCThrottle.Allow("", "jmautotest_bucket_a", 5000));
}

static void JMAutoTest_Throttle_NonPositiveIntervalAlwaysAllowed()
{
	JMAutoTest_Group("intervalMs <= 0 disables throttling for that call");

	JMAutoTest_Assert("intervalMs == 0 allowed", JMRPCThrottle.Allow("jmautotest_guid_1", "jmautotest_bucket_b", 0));
	JMAutoTest_Assert("intervalMs == 0 allowed again immediately", JMRPCThrottle.Allow("jmautotest_guid_1", "jmautotest_bucket_b", 0));
	JMAutoTest_Assert("negative intervalMs allowed", JMRPCThrottle.Allow("jmautotest_guid_1", "jmautotest_bucket_c", -1));
}

static void JMAutoTest_Throttle_SecondCallWithinWindowBlocked()
{
	JMAutoTest_Group("Second call inside the window is blocked");

	string guid = "jmautotest_guid_2";
	string bucket = "jmautotest_bucket_d";

	JMAutoTest_Assert("first call allowed", JMRPCThrottle.Allow(guid, bucket, 60000));
	JMAutoTest_AssertFalse("immediate second call blocked", JMRPCThrottle.Allow(guid, bucket, 60000));
	JMAutoTest_AssertFalse("third call still blocked", JMRPCThrottle.Allow(guid, bucket, 60000));
}

static void JMAutoTest_Throttle_DifferentBucketsAreIndependent()
{
	JMAutoTest_Group("Buckets are independent per guid");

	string guid = "jmautotest_guid_3";

	JMAutoTest_Assert("bucket X first call allowed", JMRPCThrottle.Allow(guid, "jmautotest_bucket_x", 60000));
	JMAutoTest_AssertFalse("bucket X second call blocked", JMRPCThrottle.Allow(guid, "jmautotest_bucket_x", 60000));
	// A different bucket for the SAME guid must not be affected by bucket X's cooldown.
	JMAutoTest_Assert("bucket Y first call allowed despite bucket X cooldown", JMRPCThrottle.Allow(guid, "jmautotest_bucket_y", 60000));
}

static void JMAutoTest_Throttle_ClearResetsGuid()
{
	JMAutoTest_Group("Clear() forgets a guid's windows");

	string guid = "jmautotest_guid_4";
	string bucket = "jmautotest_bucket_e";

	JMAutoTest_Assert("first call allowed", JMRPCThrottle.Allow(guid, bucket, 60000));
	JMAutoTest_AssertFalse("second call blocked before Clear()", JMRPCThrottle.Allow(guid, bucket, 60000));

	JMRPCThrottle.Clear(guid);

	JMAutoTest_Assert("call allowed again immediately after Clear()", JMRPCThrottle.Allow(guid, bucket, 60000));
}

static void JMAutoTest_Throttle_ClearDoesNotAffectOtherGuids()
{
	JMAutoTest_Group("Clear(guid) only clears that guid, not others");

	string guidA = "jmautotest_guid_5a";
	string guidB = "jmautotest_guid_5b";
	string bucket = "jmautotest_bucket_f";

	JMAutoTest_Assert("guidA first call allowed", JMRPCThrottle.Allow(guidA, bucket, 60000));
	JMAutoTest_Assert("guidB first call allowed", JMRPCThrottle.Allow(guidB, bucket, 60000));

	JMRPCThrottle.Clear(guidA);

	JMAutoTest_Assert("guidA allowed again after its own Clear()", JMRPCThrottle.Allow(guidA, bucket, 60000));
	JMAutoTest_AssertFalse("guidB still throttled - Clear(guidA) did not touch it", JMRPCThrottle.Allow(guidB, bucket, 60000));
}

static void JMAutoTest_JMRPCThrottle()
{
	PrintFormat("");
	PrintFormat("================================================================");
	PrintFormat("  AUTOTEST: JMRPCThrottle rate limiting");
	PrintFormat("================================================================");

	JMAutoTest_Throttle_EmptyGuidAlwaysAllowed();
	JMAutoTest_Throttle_NonPositiveIntervalAlwaysAllowed();
	JMAutoTest_Throttle_SecondCallWithinWindowBlocked();
	JMAutoTest_Throttle_DifferentBucketsAreIndependent();
	JMAutoTest_Throttle_ClearResetsGuid();
	JMAutoTest_Throttle_ClearDoesNotAffectOtherGuids();
}

#endif
