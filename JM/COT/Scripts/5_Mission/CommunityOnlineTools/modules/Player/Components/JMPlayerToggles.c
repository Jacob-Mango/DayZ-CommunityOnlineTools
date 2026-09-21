//! The built-in JMPlayerToggle implementations. Only what is specific to each
//! toggle is here - how it is applied to the entity and how it reads back;
//! wording, RPC and permission are supplied where the toggle is registered, in
//! JMPlayerModule.RegisterToggles().
//!
//! Kept together in one file because each is a handful of lines. A toggle a mod
//! adds should live in its own file.

class JMPlayerToggleBloodyHands: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.SetBloodyHands( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasBloodyHands(); }
}

class JMPlayerToggleGodMode: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetGodMode( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasGodMode(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTHasGodMode(); }
}

class JMPlayerToggleFreeze: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetFreeze( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.IsFrozen(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTIsFrozen(); }
}

#ifndef DAYZ_1_29
class JMPlayerToggleRagdoll: JMPlayerToggle
{
	override bool IsInUI() { return false; }
	override void Apply( PlayerBase player, int value ) { player.COTSetRagdoll( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.IsRagdoll(); }
}
#endif

class JMPlayerToggleReceiveDamageDealt: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetReceiveDamageDealt( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.GetReceiveDmgDealt(); }
}

class JMPlayerToggleCannotBeTargetedByAI: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetCannotBeTargetedByAI( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.GetCannotBeTargetedByAI(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTGetCannotBeTargetedByAI(); }
}

//! The one toggle whose value is not a bool: COTSetInvisibility takes a mode.
class JMPlayerToggleInvisibility: JMPlayerToggle
{
	//! The notification key says INVISIBILITY where the checkbox and tooltip say INVISIBLE.
	override string GetLabelKey() { return "#STR_COT_PLAYER_MODULE_TOGGLE_INVISIBILITY"; }
	override void Apply( PlayerBase player, int value ) { player.COTSetInvisibility( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasInvisibility(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTIsInvisible(); }

	override void WriteValue( ParamsWriteContext ctx, int value )
	{
		ctx.Write( value );
	}

	override bool ReadValue( ParamsReadContext ctx, out int value )
	{
		return ctx.Read( value );
	}
}

class JMPlayerToggleRemoveCollision: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetRemoveCollision( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.GetRemoveCollision(); }
}

class JMPlayerToggleUnlimitedAmmo: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetUnlimitedAmmo( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasUnlimitedAmmo(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTHasUnlimitedAmmo(); }
}

class JMPlayerToggleAdminNVG: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetAdminNVG( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasAdminNVG(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTHasAdminNVG(); }
}

class JMPlayerToggleUnlimitedStamina: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value ) { player.COTSetUnlimitedStamina( value ); }
	override int Read( JMPlayerInstance instance ) { return instance.HasUnlimitedStamina(); }
	override int ReadEntity( PlayerBase player, JMPlayerInstance instance ) { return player.COTHasUnlimitedStamina(); }
}

class JMPlayerToggleBrokenLegs: JMPlayerToggle
{
	override void Apply( PlayerBase player, int value )
	{
		if ( value )
		{
			if ( player.GetModifiersManager().IsModifierActive( eModifiers.MDF_BROKEN_LEGS ) )//effectively resets the modifier
			{
				player.GetModifiersManager().DeactivateModifier( eModifiers.MDF_BROKEN_LEGS );
			}
			player.GetModifiersManager().ActivateModifier( eModifiers.MDF_BROKEN_LEGS );
		}
		else
		{
			player.SetHealth("RightLeg", "Health", 100);
			player.SetHealth("LeftLeg", "Health", 100);
			player.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);
		}
	}

	override int Read( JMPlayerInstance instance ) { return instance.HasBrokenLegs(); }
}
