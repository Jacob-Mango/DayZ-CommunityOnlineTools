// Maintient une liste statique de tous les animaux (comme COT le fait pour les vehicules
// via CarScript.s_JM_AllCars). Necessaire car DayZ n'expose pas de liste globale d'animaux.
modded class AnimalBase
{
	static ref CF_DoublyLinkedNodes_WeakRef<AnimalBase> s_GMA_AllAnimals = new CF_DoublyLinkedNodes_WeakRef<AnimalBase>();
	ref CF_DoublyLinkedNode_WeakRef<AnimalBase> s_GMA_Node;

	void AnimalBase()
	{
		s_GMA_Node = s_GMA_AllAnimals.Add(this);
	}

	void ~AnimalBase()
	{
		if (s_GMA_AllAnimals)
			s_GMA_AllAnimals.Remove(s_GMA_Node);
	}
}
