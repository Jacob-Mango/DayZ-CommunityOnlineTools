#ifdef EXPANSIONMODVEHICLE
modded class ExpansionVehicleCover
{
	static ref CF_DoublyLinkedNodes_WeakRef<ExpansionVehicleCover> s_JM_AllCovers = new CF_DoublyLinkedNodes_WeakRef<ExpansionVehicleCover>();

	ref CF_DoublyLinkedNode_WeakRef<ExpansionVehicleCover> s_JM_Node;

	void ExpansionVehicleCover()
	{
		s_JM_Node = s_JM_AllCovers.Add(this);
	}

	void ~ExpansionVehicleCover()
	{
		if (s_JM_AllCovers)
			s_JM_AllCovers.Remove(s_JM_Node);
	}
}
#endif
