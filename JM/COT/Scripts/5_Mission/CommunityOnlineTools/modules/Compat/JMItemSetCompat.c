/**
 * Stubs for the removed ItemSetSpawner: Expansion's `modded class JMItemSetSettings`
 * (#ifdef JM_COT) fails to compile without them. Data only. See docs/systems/mod-compatibility.md.
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
