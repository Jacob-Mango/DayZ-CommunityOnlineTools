class WidgetHandler: ScriptedWidgetEventHandler
{
    private ref static WidgetHandler s_instance;

    protected ref map<Widget, ref Param> m_OnMouseButtonDownRegister;
    protected ref map<Widget, ref Param> m_OnMouseButtonUpRegister;
    protected ref map<Widget, ref Param> m_OnMouseWheel;
    protected ref map<Widget, ref Param> m_OnDropReceived;
    protected ref map<Widget, ref Param> m_OnDrag;
    protected ref map<Widget, ref Param> m_OnDrop;
    protected ref map<Widget, ref Param> m_OnDraggingOver;
    protected ref map<Widget, ref Param> m_OnMouseEnter;
    protected ref map<Widget, ref Param> m_OnMouseButtonLeave;
    protected ref map<Widget, ref Param> m_OnChange;
    protected ref map<Widget, ref Param> m_OnClick;
    protected ref map<Widget, ref Param> m_OnDoubleClick;
    protected ref map<Widget, ref Param> m_OnFocus;
    protected ref map<Widget, ref Param> m_OnFocusLost;
    protected ref map<Widget, ref Param> m_OnController;
    protected ref map<Widget, ref Param> m_OnUpdate;

    static WidgetHandler GetInstance()
    {
        if ( s_instance == NULL )
        {
            s_instance = new ref WidgetHandler;
        }
        return s_instance;
    }

    void WidgetHandler()
    {
        m_OnMouseButtonDownRegister = new map<Widget, ref Param>;
        m_OnMouseButtonUpRegister = new map<Widget, ref Param>;
        m_OnMouseWheel = new map<Widget, ref Param>;
        m_OnDropReceived = new map<Widget, ref Param>;
        m_OnDrag = new map<Widget, ref Param>;
        m_OnDrop = new map<Widget, ref Param>;
        m_OnDraggingOver = new map<Widget, ref Param>;
        m_OnMouseEnter = new map<Widget, ref Param>;
        m_OnMouseButtonLeave = new map<Widget, ref Param>;
        m_OnChange = new map<Widget, ref Param>;
        m_OnClick = new map<Widget, ref Param>;
        m_OnDoubleClick = new map<Widget, ref Param>;
        m_OnFocus = new map<Widget, ref Param>;
        m_OnFocusLost = new map<Widget, ref Param>;
        m_OnController = new map<Widget, ref Param>;
        m_OnUpdate = new map<Widget, ref Param>;
    }
    
    void UnregisterWidget( Widget w )
    {
        m_OnMouseButtonDownRegister.Remove( w );
        m_OnMouseButtonUpRegister.Remove( w );
        m_OnMouseWheel.Remove( w );
        m_OnDropReceived.Remove( w );
        m_OnDrag.Remove( w );
        m_OnDrop.Remove( w );
        m_OnDraggingOver.Remove( w );
        m_OnMouseEnter.Remove( w );
        m_OnMouseButtonLeave.Remove( w );
        m_OnChange.Remove( w );
        m_OnClick.Remove( w );
        m_OnDoubleClick.Remove( w );
        m_OnDoubleClick.Remove( w );
        m_OnFocus.Remove( w );
        m_OnFocusLost.Remove( w );
        m_OnController.Remove( w );
        m_OnUpdate.Remove( w );
        w.SetHandler( NULL );
    }

    void RegisterOnMouseButtonDown( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnMouseButtonDownRegister.Insert( w, param );
    }
    
    void RegisterOnUpdate( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnUpdate.Insert( w, param );
    }

    void RegisterOnMouseButtonUp( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnMouseButtonUpRegister.Insert( w, param );
    }

    void RegisterOnMouseWheel( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnMouseWheel.Insert( w, param );
    }

    void RegisterOnDropReceived( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnDropReceived.Insert( w, param );
    }

    void RegisterOnDrag( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnDrag.Insert( w, param );
    }

    void RegisterOnDrop( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnDrop.Insert( w, param );
    }

    void RegisterOnDraggingOver( Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnDraggingOver.Insert( w, param );
    }

    void RegisterOnMouseEnter(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnMouseEnter.Insert( w, param );
    }

    void RegisterOnMouseLeave(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnMouseButtonLeave.Insert( w, param );
    }

    void RegisterOnChange(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnChange.Insert( w, param );
    }

    void RegisterOnClick(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnClick.Insert( w, param );
    }

    void RegisterOnDoubleClick(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnDoubleClick.Insert( w, param );
    }

    void RegisterOnFocus(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnFocus.Insert( w, param );
    }
    void RegisterOnFocusLost(Widget w, Managed eventHandler, string functionName )
    {
        w.SetHandler( this );
        Param param = new Param2<Managed, string>( eventHandler, functionName );
        m_OnFocusLost.Insert( w, param );
    }

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnChange.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnChange.Remove( w );
        }


        Param param2 = new Param4<Widget, int, int, bool>( w, x, y, finished );

        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnClick.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnClick.Remove( w );
        }


        Param param2 = new Param4<Widget, int, int, int>( w, x, y, button );

        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnDoubleClick(Widget w, int x, int y, int button)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnDoubleClick.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnDoubleClick.Remove( w );
        }

        Param param2 = new Param4<Widget, int, int, int>( w, x, y, button );

        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }
    
    override bool OnUpdate(Widget w)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnUpdate.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnUpdate.Remove( w );
        }

        Param param2 = new Param1<Widget>( w );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnMouseButtonLeave.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnMouseButtonLeave.Remove( w );
        }

        Param param2 = new Param4<Widget, Widget, int, int>( w, enterW, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnMouseEnter( Widget w, int x, int y )
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnMouseEnter.Get( w ) );
        if( param == NULL )
        {
            return false;
        }
        
        if( !param.param1 )
        {
            m_OnMouseEnter.Remove( w );
        }

        Param param2 = new Param3<Widget, int, int>( w, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnMouseButtonDown( Widget w, int x, int y, int button )
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnMouseButtonDownRegister.Get( w ) );
        if( param == NULL )
        {
            return false;
        }
        
        if( !param.param1 )
        {
            m_OnMouseButtonDownRegister.Remove( w );
        }

        Param param2 = new Param4<Widget, int, int, int>( w, x, y, button );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnMouseButtonUp( Widget w, int x, int y, int button )
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnMouseButtonUpRegister.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnMouseButtonUpRegister.Remove( w );
        }

        Param param2 = new Param4<Widget, int, int, int>( w, x, y, button );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnDrag(Widget w, int x, int y)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnDrag.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnDrag.Remove( w );
        }

        Param param2 = new Param3<Widget, int, int>( w, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnDrop(Widget w, int x, int y, Widget reciever)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnDrop.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnDrop.Remove( w );
        }

        Param param2 = new Param3<Widget, int, int>( w, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnDraggingOver(Widget w, int x, int y, Widget reciever)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnDraggingOver.Get( reciever ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnDraggingOver.Remove( w );
        }

        Param param2 = new Param4<Widget, int, int, Widget>( w, x, y, reciever );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnDropReceived( Widget w, int x, int y, Widget reciever )
    {
        if( w.GetName() == "GridItem" )
        {
            return false;
        }
        Param p = m_OnDropReceived.Get( reciever );
        Param2<Managed, string> param = Param2<Managed, string>.Cast( p );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnDropReceived.Remove( w );
        }

        Param param2 = new Param4<Widget, int, int, Widget>( w, x, y, reciever );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnFocus( Widget w, int x, int y )
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnFocus.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnFocus.Remove( w );
        }

        Param param2 = new Param3<Widget, int, int>( w, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnFocusLost( Widget w, int x, int y )
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnFocusLost.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnFocusLost.Remove( w );
        }

        Param param2 = new Param3<Widget, int, int>( w, x, y );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }

    override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
    {
        Param2<Managed, string> param = Param2<Managed, string>.Cast( m_OnMouseWheel.Get( w ) );
        if( param == NULL )
        {
            return false;
        }

        if( !param.param1 )
        {
            m_OnMouseWheel.Remove( w );
        }

        Param param2 = new Param3<int, int, int>( x, y, wheel );
        
        bool ret = false;
        GetGame().GameScript.CallFunctionParams( param.param1, param.param2, ret, param2 );
        return ret;
    }
}
