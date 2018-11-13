class EditorModule extends Module
{
    ref Form form;
    ref ButtonWidget menuButton;

    string GetLayoutRoot()
    {
        return "";
    }

    bool HasAccess()
    {
        return true;
    }
}