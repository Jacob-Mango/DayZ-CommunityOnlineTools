// =============================================================================
//  UIActionLogView
//
//  A scrollable read-only text log.  Lines are appended with AppendLine();
//  each line can have its own color.  When the buffer exceeds m_MaxLines,
//  the oldest line is dropped.  Auto-scrolls to the bottom on each append
//  unless the user has scrolled up manually.
//
//  Usage:
//      m_Log = UIActionManager.CreateLogView( parent, 100 );
//      m_Log.AppendLine( "Player joined",  ARGB(255, 100,200,100) );
//      m_Log.AppendLine( "Vehicle deleted",ARGB(255, 200,100,100) );
//      m_Log.AppendLine( "RPC received"  );   // default white
//      m_Log.Clear();
// =============================================================================

class JMLogLine
{
	string Text;
	int    Color;
	Widget Root;
}

class UIActionLogView: UIActionBase
{
	protected Widget            m_Background;
	protected GridSpacerWidget  m_Lines;
	protected ScrollWidget      m_Scroll;

	protected ref array<ref JMLogLine> m_Buffer;
	protected int  m_MaxLines;
	protected bool m_AutoScroll;

	static const int DEFAULT_LINE_COLOR = JMTheme.TEXT_SECONDARY;
	static const int LINE_HEIGHT        = 18;

	override void OnInit()
	{
		super.OnInit();

		Class.CastTo( m_Background, layoutRoot.FindAnyWidget( "action_background" ) );
		Class.CastTo( m_Scroll,     layoutRoot.FindAnyWidget( "action_scroll"     ) );
		Class.CastTo( m_Lines,      layoutRoot.FindAnyWidget( "action_lines"      ) );

		m_Buffer     = new array<ref JMLogLine>;
		m_MaxLines   = 200;
		m_AutoScroll = true;
	}

	//! Maximum number of lines kept in memory (oldest are dropped).
	void SetMaxLines( int max )
	{
		m_MaxLines = Math.Max( 1, max );
		TrimBuffer();
	}

	//! Append a line of text with optional color.
	void AppendLine( string text, int color = 0 )
	{
		if ( !m_Lines )
			return;

		if ( color == 0 )
			color = DEFAULT_LINE_COLOR;

		ref JMLogLine line = new JMLogLine();
		line.Text  = text;
		line.Color = color;

		TextWidget tw = TextWidget.Cast(
			g_Game.GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/uiactions/UIActionLogLine.layout", m_Lines ) );

		if ( tw )
		{
			tw.SetText( text );
			tw.SetColor( color );
			line.Root = tw;
		}

		m_Buffer.Insert( line );
		TrimBuffer();

		if ( m_AutoScroll && m_Scroll )
			m_Scroll.VScrollToPos01( 1.0 );
	}

	void Clear()
	{
		if ( !m_Lines )
			return;

		Widget child = m_Lines.GetChildren();
		while ( child )
		{
			Widget next = child.GetSibling();
			child.Unlink();
			child = next;
		}
		m_Buffer.Clear();
	}

	int GetLineCount()
	{
		return m_Buffer.Count();
	}

	private void TrimBuffer()
	{
		while ( m_Buffer.Count() > m_MaxLines )
		{
			JMLogLine oldest = m_Buffer[0];
			if ( oldest && oldest.Root )
				oldest.Root.Unlink();
			m_Buffer.Remove( 0 );
		}
	}


}
