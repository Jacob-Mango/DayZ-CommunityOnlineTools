// =============================================================================
//  Server-side tracking for vanilla heli crash sites (CrashBase and subclasses).
//  Mirrors the CarScript pattern: a CF doubly-linked weak-ref list keeps track
//  of every live instance without a world scan.
// =============================================================================

#ifndef EXPANSIONMODAI

modded class CrashBase
{
	static ref CF_DoublyLinkedNodes_WeakRef<CrashBase> s_JM_COT_AllCrashes = new CF_DoublyLinkedNodes_WeakRef<CrashBase>();

	ref CF_DoublyLinkedNode_WeakRef<CrashBase> s_JM_COT_Node;

	void CrashBase()
	{
		if ( GetGame().IsServer() )
			s_JM_COT_Node = s_JM_COT_AllCrashes.Add( this );
	}

	void ~CrashBase()
	{
		if ( s_JM_COT_AllCrashes )
			s_JM_COT_AllCrashes.Remove( s_JM_COT_Node );
	}
}

#endif
