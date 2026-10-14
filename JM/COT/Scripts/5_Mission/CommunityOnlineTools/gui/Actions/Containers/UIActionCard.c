//  UIActionCard: bordered container with optional title bar and padded body (card.GetContent()).
//  Title "" = no header. EXACTLY ONE Size-To-Content-V IN THE CHAIN (card_content): two nested
//  resolve circularly to zero and the card renders as a bare title bar. card_content is a GRID,
//  not a panel: a panel measures children whose heights are fractions of it, so it grows unbounded.
class UIActionCard: UIActionBase
{
	//! Must match card_header's height in the layout.
	static const int HEADER_HEIGHT = 30;

	//! Gap between the header rule and the first row, so overflowing controls do not land on
	//! the header (which is also the LAST sibling in the layout, painting over any that reach it).
	//! Must match card_content's y in the layout, which is HEADER_HEIGHT + this.
	static const int CONTENT_TOP_PADDING = 6;

	//! Breathing room under the last row so it does not sit on the border.
	static const int BOTTOM_PADDING = 8;

	//! Hard ceiling on the root height. Pure insurance: a card measuring its own descendants can
	//! feed back into itself and grow every frame; the clamp turns that into a merely wrong
	//! height. No real card (tallest is a full inventory grid) approaches it.
	static const int MAX_HEIGHT = 4000;

	//! Height the root is given before anything has been measured. The layout declares it 1px
	//! tall, where a content-sized child can measure nothing and the card would deadlock at
	//! zero. Starting tall gives the first layout room; SyncHeight then shrinks to fit.
	static const int SEED_HEIGHT = 240;
	protected Widget     m_Header;
	protected Widget     m_Content;
	protected Widget     m_Ring;
	protected TextWidget m_Title;

	//! Right-hand strip inside the title bar. A sibling of the header, not a child: card_header
	//! takes no pointer input and a child cannot opt back in. Declared after card_ring in the
	//! layout so it also draws over the border.
	protected Widget     m_HeaderActions;
	protected bool m_HasHeader;

	//! Last height pushed onto the root. Kept so the per-frame check is a float
	//! compare and not a SetSize on every card on screen every frame.
	protected float m_AppliedHeight;

	//! The widget callers put their rows into.
	Widget GetContent()
	{
		return m_Content;
	}

	//! Right-aligned title bar strip for controls acting on the whole card. Children pack toward
	//! the right in creation order (first created sits leftmost). Size them explicitly: the
	//! strip is 30px tall and gives its children no width.
	Widget GetHeaderActions()
	{
		return m_HeaderActions;
	}

	//! Tint the border. A destructive section gets a red ring rather than a red
	//! heading, so the whole block reads as dangerous and not just its name.
	void SetRingColor( int color )
	{
		if ( m_Ring )
			m_Ring.SetColor( color );
	}

	void SetTitleColor( int color )
	{
		if ( m_Title )
			m_Title.SetColor( color );
	}

	override void OnInit()
	{
		super.OnInit();

		m_Header  = layoutRoot.FindAnyWidget( "card_header"  );
		m_Content = layoutRoot.FindAnyWidget( "card_content" );
		m_Ring    = layoutRoot.FindAnyWidget( "card_ring"    );

		m_HeaderActions = layoutRoot.FindAnyWidget( "card_header_actions" );

		// Packed against the right edge and centred in the 30px strip, so a
		// caller only has to create its buttons in reading order.
		WrapSpacerWidget actions;
		if ( Class.CastTo( actions, m_HeaderActions ) )
		{
			actions.SetContentAlignmentH( WidgetAlignment.WA_RIGHT );
			actions.SetContentAlignmentV( WidgetAlignment.WA_CENTER );
		}

		Class.CastTo( m_Title, layoutRoot.FindAnyWidget( "card_title" ) );

		m_HasHeader     = true;
		m_AppliedHeight = 0;

		float rw, rh;
		layoutRoot.GetSize( rw, rh );
		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( rw, JMUILayout.Px( SEED_HEIGHT ) );
	}

	//  Convenience Premade Card Header Action Creators
	//! Every header action is the same size, so the helpers apply it and callers
	//! never resize what they get back.
	protected UIActionImageButton SizeHeaderAction( UIActionImageButton btn )
	{
		if ( btn )
			btn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );

		return btn;
	}

	UIActionImageButton AddRefreshButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_REFRESH" )
	{
		return SizeHeaderAction( UIActionManager.CreateRefreshButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddDeleteButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_DELETE" )
	{
		return SizeHeaderAction( UIActionManager.CreateDeleteButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddSaveButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_SAVE" )
	{
		return SizeHeaderAction( UIActionManager.CreateSaveButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddApplyButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_APPLY" )
	{
		return SizeHeaderAction( UIActionManager.CreateApplyButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddCopyButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_COPY" )
	{
		return SizeHeaderAction( UIActionManager.CreateCopyButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddPasteButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_PASTE" )
	{
		return SizeHeaderAction( UIActionManager.CreatePasteButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddAddButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_ADD" )
	{
		return SizeHeaderAction( UIActionManager.CreateAddButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddEditButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_EDIT" )
	{
		return SizeHeaderAction( UIActionManager.CreateEditButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddSearchButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_SEARCH" )
	{
		return SizeHeaderAction( UIActionManager.CreateSearchButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddLockButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_LOCK" )
	{
		return SizeHeaderAction( UIActionManager.CreateLockButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	UIActionImageButton AddVisibilityButton( Class instance, string callback, string tooltip = "#STR_COT_GENERIC_VIEW" )
	{
		return SizeHeaderAction( UIActionManager.CreateVisibilityButton( GetHeaderActions(), instance, callback, tooltip ) );
	}

	//! Escape hatch behind the named helpers above: a header action with an arbitrary Lucide icon.
	//! Prefer a named helper where one fits, so icon and tooltip stay consistent across modules.
	//!   card.AddCardHeaderAction( JMConstants.Lucide( "download" ), this, "OnClick_Export", "#STR_..." );
	UIActionImageButton AddCardHeaderAction( string iconPath, Class instance, string callback, string tooltip = "" )
	{
		UIActionImageButton btn = UIActionManager.CreateIconButton( GetHeaderActions(), iconPath, instance, callback );

		if ( btn )
		{
			btn.SetFixedSize( JMFormBase.HEADER_ACTION_PX, JMFormBase.HEADER_ACTION_PX );

			if ( tooltip != "" )
				btn.SetTooltip( tooltip );
		}

		return btn;
	}

	//! Take a button added with AddCardHeaderAction / Add*Button out of the title bar.
	void RemoveCardHeaderAction( UIActionImageButton button )
	{
		if ( button && button.GetLayoutRoot() )
			button.GetLayoutRoot().Unlink();
	}

	override void SetLabel( string text )
	{
		m_HasHeader = ( text != "" );

		if ( m_Header )
			m_Header.Show( m_HasHeader );

		// A headerless card has no title bar to hang controls in.
		if ( m_HeaderActions )
			m_HeaderActions.Show( m_HasHeader );

		// A headerless card starts its body at the top edge instead of leaving
		// an empty strip where the title bar would have been.
		if ( m_Content )
		{
			if ( m_HasHeader )
				m_Content.SetPos( 0, JMUILayout.Px( HEADER_HEIGHT + CONTENT_TOP_PADDING ), true );
			else
				m_Content.SetPos( 0, JMUILayout.Px( CONTENT_TOP_PADDING ), true );
		}

		if ( m_HasHeader && m_Title )
			m_Title.SetText( text );
	}

	override void Update( float timeSlice )
	{
		super.Update( timeSlice );

		SyncHeight();
	}

	//! Give the root a height: the header, plus whatever card_content laid out,
	//! plus the same margin at the bottom that the header gives at the top.
	protected void SyncHeight()
	{
		if ( !layoutRoot || !m_Content )
			return;

		float cw, ch;
		m_Content.GetScreenSize( cw, ch );

		if ( ch <= 0 )
			return;

		float height = ch + JMUILayout.Px( CONTENT_TOP_PADDING + BOTTOM_PADDING );
		if ( m_HasHeader )
			height = height + JMUILayout.Px( HEADER_HEIGHT );

		// Float compare against the last value we set, not against the root's
		// current size: reading the root back gives layout units, and these are
		// physical pixels.
		if ( Math.AbsFloat( height - m_AppliedHeight ) < 1 )
			return;

		height = Math.Clamp( height, 0, JMUILayout.Px( MAX_HEIGHT ) );

		m_AppliedHeight = height;

		float rw, rh;
		layoutRoot.GetSize( rw, rh );

		layoutRoot.SetFlags( WidgetFlags.VEXACTSIZE, true );
		layoutRoot.SetSize( rw, height );
		layoutRoot.Update();
	}

	override void UpdatePermission( string permission )
	{
		super.UpdatePermission( permission );

		if ( layoutRoot && layoutRoot.IsVisible() )
		{
			CheckChildVisibility();
		}
	}

	void CheckChildVisibility()
	{
		if ( !m_Content )
			return;

		Widget child = m_Content.GetChildren();
		if ( child )
		{
			bool anyVisible = false;
			while ( child )
			{
				if ( child.IsVisible() )
				{
					anyVisible = true;
					break;
				}
				child = child.GetSibling();
			}

			if ( !anyVisible )
			{
				layoutRoot.Show( false );
			}
		}
	}
}
