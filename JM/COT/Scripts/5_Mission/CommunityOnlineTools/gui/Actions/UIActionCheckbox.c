// =============================================================================
//  UIActionCheckbox
//
//  Thin alias over UIActionToggle. Historically backed by the vanilla
//  CheckBoxWidget; now uses the custom toggle visuals so every checkbox in
//  the mod shares the same look & feel.
//
//  All existing call sites using UIActionManager.CreateCheckbox and
//  UIActionCheckbox.SetChecked/IsChecked keep working unchanged.
// =============================================================================
class UIActionCheckbox: UIActionToggle
{
}
