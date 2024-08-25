class JMMapEditorForm 
{
	protected Widget layoutRoot;

	protected EntityAI m_SelectedObject;
	protected float m_Distance = 200.0;

	protected ref JMMapEditorModule m_Module;

	protected bool m_IsShown;
	protected bool m_MouseButtonPressed;
	protected int m_PreviousTime;

	protected float m_MoveSensitivity;
	protected float m_MouseSensitivity;

	protected float m_DistanceFromObject;

	void JMMapEditorForm( JMMapEditorModule module )
	{
		m_Module = module;
		m_MouseButtonPressed = false;
	}
	
	void ~JMMapEditorForm()
	{
		Hide();
	}

	bool IsVisible()
	{
		return layoutRoot.IsVisible();
	}
	
	Widget Init()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Init" );
		#endif

		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "JM/COT/GUI/layouts/mapeditor_form.layout" );
		layoutRoot.Show( false );

		Widget objectInfoWrapper = layoutRoot.FindAnyWidget( "object_info_wrapper" );

		Widget objectInfoGrid = UIActionManager.CreateGridSpacer( objectInfoWrapper, 1, 2 );

		UIActionManager.CreateEditableVector( objectInfoGrid, "Position: " );
		UIActionManager.CreateEditableVector( objectInfoGrid, "Rotation: " );

		Widget objectControlsWrapper = layoutRoot.FindAnyWidget( "object_controls" );

		Widget objectControlsGrid = UIActionManager.CreateGridSpacer( objectControlsWrapper, 1, 2 );
		UIActionManager.CreateText( objectControlsGrid, "Select: ", "Left Mouse" );
		UIActionManager.CreateText( objectControlsGrid, "Delete: ", "Delete" );

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Init" );
		#endif

		return layoutRoot;
	}

	void Show()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Show" );
		#endif

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

		if (m_IsShown)
			return;

		layoutRoot.Show( true );

		m_PreviousTime = GetGame().GetTime();

		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Insert( this.Update );

		OnShow();

		m_IsShown = true;

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Show" );
		#endif
	}

	void Hide()
	{
		#ifdef COT_DEBUGLOGS
		Print( "+" + this + "::Hide" );
		#endif

		if ( GetGame().IsServer() && GetGame().IsMultiplayer() ) return;

		layoutRoot.Show( false );

		GetGame().GetUpdateQueue(CALL_CATEGORY_GUI).Remove( this.Update );

		OnHide();

		m_IsShown = false;

		#ifdef COT_DEBUGLOGS
		Print( "-" + this + "::Hide" );
		#endif
	}

	void Update()
	{
		int currentTime = GetGame().GetTime();
		OnUpdate( ( m_PreviousTime - currentTime ) / 1000.0);
		m_PreviousTime = currentTime;
	}

	void OnShow()
	{
	}

	void OnHide()
	{
	}

	void CheckForSelection()
	{
	}

	void OnUpdate( float timeslice )
	{
		if ( !CurrentActiveCamera )
			return;

		Input input = GetGame().GetInput();

		if ( GetWidgetUnderCursor() == NULL || GetWidgetUnderCursor().GetName() != "Windows_Container" && GetWidgetUnderCursor().GetName() != "map_editor_menu" )
			return;
		
		if ( input.LocalRelease( "UADefaultAction", false ) && m_SelectedObject )
		{
			m_SelectedObject = NULL;
		}

		if ( input.LocalPress( "UADefaultAction", false ) )
		{
			m_SelectedObject = EntityAI.Cast( GetPointerObject( m_Distance ) );

			if ( GetGame().GetPlayer() && vector.Distance( m_SelectedObject.GetPosition(), GetGame().GetPlayer().GetPosition() ) > 900.0 )
				m_SelectedObject = NULL;

			if ( m_SelectedObject && !m_SelectedObject.HasNetworkID() )
				m_SelectedObject = NULL;

			if ( m_SelectedObject )
			{
				m_DistanceFromObject = vector.Distance( CurrentActiveCamera.GetPosition(), m_SelectedObject.GetPosition() );
			}
		}

		if ( m_SelectedObject )
		{
			CurrentActiveCamera.MoveFreeze = true;
			CurrentActiveCamera.LookFreeze = true;

			if ( input.LocalValue( "UATurbo" ) > 0 )
			{
				m_MoveSensitivity = 60.0;
				m_MouseSensitivity = 10.0;
			} else
			{
				m_MoveSensitivity = 10.0;
				m_MouseSensitivity = 1.0;
			}

			vector position = m_SelectedObject.GetPosition();
			vector orientation = m_SelectedObject.GetOrientation();

			vector forwardDir = CurrentActiveCamera.GetDirection();
			forwardDir[1] = 0;
			forwardDir.Normalize();
			vector right = vector.Up * CurrentActiveCamera.GetDirection();
			vector up = vector.Up;

			float forward = input.LocalValue( "UAMoveForward" ) - input.LocalValue( "UAMoveBack" );
			float strafe = input.LocalValue( "UAMoveRight" ) - input.LocalValue( "UAMoveLeft" );
			float altitude = input.LocalValue( "UALeanLeft" ) - input.LocalValue( "UALeanRight" );

			float yawDiff = input.LocalValue( "UAAimLeft" ) - input.LocalValue( "UAAimRight" );
			float pitchDiff = input.LocalValue( "UAAimDown" ) - input.LocalValue( "UAAimUp" );

			float distanceInc = input.LocalValue( "UACameraToolSpeedIncrease" ) - input.LocalValue( "UACameraToolSpeedDecrease" );

			// float changeInHeight = position[0];

			position = position + ( forwardDir * timeslice * m_MoveSensitivity * forward );
			position = position + ( right * timeslice * m_MoveSensitivity * strafe );
			position = position + ( up * timeslice * m_MoveSensitivity * altitude );

			orientation[0] = orientation[0] + ( timeslice * yawDiff * m_MouseSensitivity );
			orientation[1] = orientation[1] + ( timeslice * pitchDiff * m_MouseSensitivity );

			m_DistanceFromObject = m_DistanceFromObject + distanceInc;

			if ( m_DistanceFromObject < 0 )
				m_DistanceFromObject = 0;

			if ( m_DistanceFromObject > m_Distance )
				m_DistanceFromObject = m_Distance;

			CurrentActiveCamera.SetPosition( position + ( CurrentActiveCamera.GetDirection() * -m_DistanceFromObject ) );

			CurrentActiveCamera.LookAt( position );

			if ( !GetGame().GetPlayer() || vector.Distance( position, GetGame().GetPlayer().GetPosition() ) < 900.0 )
			{
				m_SelectedObject.SetPosition( position );
				m_SelectedObject.SetOrientation( orientation );	
			}
		} else 
		{
			CurrentActiveCamera.MoveFreeze = false;
			CurrentActiveCamera.LookFreeze = false;
		}
	}
};
