//  Server-side tracking for vanilla heli crash sites (CrashBase and subclasses).
//  Mirrors the CarScript pattern: a CF weak-ref linked list tracks every live
//  instance without a world scan.

#ifndef EXPANSIONMODAI

modded class CrashBase
{
	static ref CF_DoublyLinkedNodes_WeakRef<CrashBase> s_JM_COT_AllCrashes = new CF_DoublyLinkedNodes_WeakRef<CrashBase>();
	ref CF_DoublyLinkedNode_WeakRef<CrashBase> s_JM_COT_Node;

	//! We are only interested in crashes spawned by events, not random static objs part of the map
	//! Server-only
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		s_JM_COT_Node = s_JM_COT_AllCrashes.Add(this);
	}

	//! We are only interested in crashes spawned by events, not random static objs part of the map
	//! Server-only
	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();

		s_JM_COT_Node = s_JM_COT_AllCrashes.Add(this);
	}

	void ~CrashBase()
	{
	#ifndef DIAG_DEVELOPER
		if (!g_Game)
			return;
	#endif

		if (s_JM_COT_AllCrashes)
			s_JM_COT_AllCrashes.Remove(s_JM_COT_Node);
	}
}

#endif
