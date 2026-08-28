// Maintient une liste statique de toutes les constructions joueurs, exactement comme
// COT le fait pour les vehicules (CarScript.s_JM_AllCars) ou comme COT_AnimalManager
// le fait pour les animaux (AnimalBase.s_GMA_AllAnimals).
//
// BaseBuildingBase est la classe globale dont herite la quasi-totalite du base building
// (cloture, mirador, mat de territoire, portail...) ainsi que la plupart des pieces de
// base provenant des mods. C'est elle que l'ESP natif de COT utilise pour "Base Building".
//
// TentBase est ajoutee en plus car les tentes sont des constructions persistantes que les
// admins veulent generalement voir et gerer comme le reste de la base.

modded class BaseBuildingBase
{
	static ref CF_DoublyLinkedNodes_WeakRef<BaseBuildingBase> s_COT_AllBuildings = new CF_DoublyLinkedNodes_WeakRef<BaseBuildingBase>();
	ref CF_DoublyLinkedNode_WeakRef<BaseBuildingBase> s_COT_BuildNode;

	void BaseBuildingBase()
	{
		s_COT_BuildNode = s_COT_AllBuildings.Add(this);
	}

	void ~BaseBuildingBase()
	{
		if (s_COT_AllBuildings)
			s_COT_AllBuildings.Remove(s_COT_BuildNode);
	}
}

modded class TentBase
{
	static ref CF_DoublyLinkedNodes_WeakRef<TentBase> s_COT_AllTents = new CF_DoublyLinkedNodes_WeakRef<TentBase>();
	ref CF_DoublyLinkedNode_WeakRef<TentBase> s_COT_TentNode;

	void TentBase()
	{
		s_COT_TentNode = s_COT_AllTents.Add(this);
	}

	void ~TentBase()
	{
		if (s_COT_AllTents)
			s_COT_AllTents.Remove(s_COT_TentNode);
	}
}
