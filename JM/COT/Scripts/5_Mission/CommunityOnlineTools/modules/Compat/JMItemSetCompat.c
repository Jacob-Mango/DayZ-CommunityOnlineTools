/**
 * JMItemSetCompat.c
 *
 * Compatibility stubs for the removed ItemSetSpawner module.
 *
 * COT's ItemSetSpawner module (form, module, RPC and JSON persistence) was
 * deleted in 893b62e5. DayZ-Expansion still ships
 * `modded class JMItemSetSettings` behind `#ifdef JM_COT`, so without these
 * types Expansion fails to compile whenever COT is loaded:
 *
 *   dayzexpansion\classes\modules\itemsets\jmitemsetsettings.c(14):
 *   Unknown type 'JMItemSetSettings'
 *
 * These are data carriers only. Nothing in COT constructs or reads them --
 * Expansion's override populates ItemSets and no consumer ever drains it.
 * The persistence half of the original API (Load/Save/Create) is intentionally
 * absent: it depended on JMConstants.DIR_ITEMS / EXT_ITEM, which were removed
 * with the module. Expansion never calls it.
 *
 * See docs/systems/mod-compatibility.md.
 */

class JMItemSetItemInfo
{
	string ItemName;
	int NumberOfStacks;
	int StackSize;

	void JMItemSetItemInfo( string item, int numStacks, int stackSize )
	{
		ItemName = item;
		NumberOfStacks = numStacks;
		StackSize = stackSize;
	}
}

class JMItemSetSerialize
{
	[NonSerialized()]
	string m_FileName;

	string Name;
	string ContainerClassName;

	ref array< ref JMItemSetItemInfo > Items;

	void JMItemSetSerialize()
	{
		Items = new array< ref JMItemSetItemInfo >;
	}

	//! Expansion's Defaults() builds sets via `new JMItemSetSerialize`, but the
	//! original COT defaults used this factory. Kept so either form compiles.
	static JMItemSetSerialize Create()
	{
		return new JMItemSetSerialize;
	}
}

class JMItemSetSettings
{
	ref map< string, ref JMItemSetSerialize > ItemSets;

	//! protected, not private: DayZ-Expansion declares
	//! `modded class JMItemSetSettings`, and a modded class cannot reach a
	//! private member of the class it mods -- including its constructor.
	//! The original was private, which alone would break Expansion's compile.
	protected void JMItemSetSettings()
	{
		ItemSets = new map< string, ref JMItemSetSerialize >;
	}

	//! Expansion overrides this and calls super.Defaults() first. COT no longer
	//! ships any default sets of its own, so the base body is empty.
	void Defaults()
	{
	}
}
