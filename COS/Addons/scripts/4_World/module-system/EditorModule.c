class EditorModule extends Module
{
    // Only accessed by EditorMenu class. Don't use in module.
    ref PopupMenu m_MenuPopup;
    ref Widget m_Menu;
    ref ButtonWidget m_Button;

    string GetLayoutRoot()
    {
        return "";
    }
}