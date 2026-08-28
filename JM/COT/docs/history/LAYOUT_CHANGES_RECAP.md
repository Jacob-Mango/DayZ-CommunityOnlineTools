# Layout Changes Recap

This file summarizes the layout-focused UI work currently present in the project working tree, with extra detail for every `.layout` file that was changed, added, renamed, or removed.

## Overall Direction

The layout work moves COT toward a shared visual language:

- Shared COT surfaces replace many `DayZDefaultPanel` usages.
- Window and sidebar chrome now use COT-owned `.paa` icons instead of text glyphs or built-in DayZ icon references.
- Reusable `UIAction` layouts were expanded so module forms can build richer controls without hand-crafting each screen.
- Several old or special-case layouts were removed after newer consolidated layouts replaced them.

## Repeated Surface-Style Changes

The most common layout change was:

```text
style DayZDefaultPanel
```

changed to:

```text
style COTSurface
```

This was applied where the widget represents a COT-owned surface, form shell, panel, row background, or list entry. The intent is to route the visual surface through `GUI/styles/cot.styles` instead of using the older DayZ default panel style directly.

Files changed only or mostly for this surface migration:

- `GUI/layouts/Example_form.layout`
  - Root `PanelWidgetClass JMExampleForm` now uses `COTSurface`.
- `GUI/layouts/ItemStatsForm.layout`
  - Root `PanelWidgetClass JMItemStatsForm` now uses `COTSurface`.
- `GUI/layouts/ban_form.layout`
  - Root `PanelWidgetClass JMBanForm` now uses `COTSurface`.
- `GUI/layouts/camera_form.layout`
  - Root `PanelWidgetClass JMCameraForm` now uses `COTSurface`.
- `GUI/layouts/esp_form.layout`
  - Root `PanelWidgetClass JMESPForm` now uses `COTSurface`.
- `GUI/layouts/eventspawner_form.layout`
  - Root `PanelWidgetClass JMNamalskEventManagerForm` now uses `COTSurface`.
- `GUI/layouts/itemsetspawner_form.layout`
  - Root `PanelWidgetClass JMItemSetForm` now uses `COTSurface`.
- `GUI/layouts/loadout_form.layout`
  - Root `PanelWidgetClass JMLoadoutForm` now uses `COTSurface`.
- `GUI/layouts/mapview_form.layout`
  - Root `FrameWidgetClass COTMap` now uses `COTSurface`.
- `GUI/layouts/mission_selector_form.layout`
  - Root `PanelWidgetClass JMMissionSelectorForm` now uses `COTSurface`.
- `GUI/layouts/objectspawner_form.layout`
  - Root `PanelWidgetClass JMObjectSpawnerForm` now uses `COTSurface`.
- `GUI/layouts/serverinfo_form.layout`
  - Root `GridSpacerWidgetClass JMServerInfoForm` now uses `COTSurface`.
- `GUI/layouts/sidebar_menu.layout`
  - Sidebar title/body surface now uses `COTSurface`.
- `GUI/layouts/teleport_form.layout`
  - Root `PanelWidgetClass JMTeleportForm` now uses `COTSurface`.
- `GUI/layouts/vehiclespawner_form.layout`
  - Root `PanelWidgetClass JMVehicleSpawnerForm` now uses `COTSurface`.
- `GUI/layouts/webhook_form.layout`
  - Root `PanelWidgetClass JMWebhookForm` now uses `COTSurface`.
- `GUI/layouts/vehicles/Vehicles_List_Entry_New.layout`
  - Vehicle list entry surface now uses `COTSurface`.
- `GUI/layouts/permission_widget.layout`
  - Permission row background panel now uses `COTSurface`.
- `GUI/layouts/role_widget.layout`
  - Role row background panel now uses `COTSurface`.

## Detailed Existing Layout Changes

### `GUI/layouts/windowbase.layout`

The shared COT window base was refreshed.

- `background` changed from `DayZDefaultPanel` to `COTSurface`.
- `confirmation_wrapper` changed from `DayZDefaultPanel` to `COTSurface`.
- `title_bar_drag` changed from `DayZDefaultPanel` to `COTSurface`.
- `title_wrapper` width changed from `1` to `0.86`, leaving explicit space for the title-bar controls.
- `pin_button_label` changed from a text widget to an image widget:
  - Old: `TextWidgetClass pin_button_label`
  - New: `ImageWidgetClass pin_button_label`
  - Old text value `P` was removed.
  - New icon source: `JM/COT/GUI/textures/icons/pin.paa`
  - New fixed icon size: `16 16`
  - New alignment: centered horizontally and vertically.
  - New render mode: `blend`
  - Added alpha, no-wrap, clamp, and stretch settings.
- `minimize_button_label` changed from a text widget to an image widget:
  - Old: `TextWidgetClass minimize_button_label`
  - New: `ImageWidgetClass minimize_button_label`
  - Old text value `_` was removed.
  - New icon source: `JM/COT/GUI/textures/icons/expand.paa`
  - New fixed icon size: `16 16`
  - New alignment: centered horizontally and vertically.
  - New render mode: `blend`
  - Added alpha, no-wrap, clamp, and stretch settings.
- `close_button_image` changed from a full-size DayZ icon to a fixed COT icon:
  - Old source: `set:dayz_gui image:icon_x`
  - New source: `JM/COT/GUI/textures/icons/close.paa`
  - Old render mode: `additive`
  - New render mode: `blend`
  - Old size: `1 1`
  - New size: `16 16`
  - Alignment changed from right-aligned to centered.
  - Exact position and exact size flags were enabled for predictable icon sizing.

### `GUI/layouts/confirmation.layout`

The confirmation dialog was enlarged and rebalanced.

- Root `confirmation_panel_inner` size changed:
  - Old: `340 220`
  - New: `440 300`
- Root style changed from `DayZDefaultPanel` to `COTSurface`.
- `confirmation_title_panel` was moved and shortened:
  - Position changed from `0 0.04` to `0 0.03`.
  - Size changed from `0.95 0.22` to `0.95 0.16`.
- `confirmation_spacer_title_message` moved upward:
  - Position changed from `0 0.27` to `0 0.20`.
  - Style changed from `DayZDefaultPanel` to `COTSurface`.
- `confirmation_message_panel` was given more vertical space:
  - Position changed from `0 0.28` to `0 0.22`.
  - Size changed from `0.95 0.44` to `0.95 0.53`.
- `confirmation_buttons_panel` was moved and shortened:
  - Position changed from `0 0.04` to `0 0.03`.
  - Size changed from `0.95 0.22` to `0.95 0.17`.

### `GUI/layouts/mapeditor_form.layout`

The map editor bottom panels were made responsive horizontally.

- `object_info_wrapper`:
  - Position changed from exact pixels `20 80` to fractional horizontal placement `0.02 80`.
  - Size changed from fixed width `800 40` to fractional width `0.96 40`.
  - `hexactpos` changed from `1` to `0`.
  - `hexactsize` changed from `1` to `0`.
  - Style changed from `DayZDefaultPanel` to `COTSurface`.
- `object_controls`:
  - Position changed from exact pixels `20 20` to fractional horizontal placement `0.02 20`.
  - Size changed from fixed width `800 40` to fractional width `0.96 40`.
  - `hexactpos` changed from `1` to `0`.
  - `hexactsize` changed from `1` to `0`.
  - Style changed from `DayZDefaultPanel` to `COTSurface`.

### `GUI/layouts/mission_form.layout`

The mission selector shell and its internal split panels were moved onto the COT surface style.

- Root `JMMissionSelectorForm` style changed to `COTSurface`.
- `panel_left` style changed to `COTSurface`.
- `panel_right` style changed to `COTSurface`.
- Nested `panel_split` style changed to `COTSurface`.
- Existing panel sizes, positions, colors, and split behavior were otherwise preserved.

### `GUI/layouts/player_form.layout`

The player form shell and split panels were moved onto the COT surface style.

- Root `JMPlayerForm` style changed to `COTSurface`.
- `panel_left` style changed to `COTSurface`.
- `panel_right` style changed to `COTSurface`.
- Nested `panel_split` style changed to `COTSurface`.
- Existing left/right layout proportions were preserved.

### `GUI/layouts/role_manager_form.layout`

The role manager shell and left panel were moved onto the COT surface style.

- Root `JMRoleManagerForm` style changed to `COTSurface`.
- `panel_left` style changed to `COTSurface`.
- The existing two-panel layout remained intact.

### `GUI/layouts/weather_form.layout`

The weather form surface and list scroller positioning were adjusted.

- Root `JMWeatherForm` style changed to `COTSurface`.
- `panel_left` style changed to `COTSurface`.
- `list_scroller` was moved down to make room for content above it:
  - Position changed from `0 5` to `0 35`.
  - Size changed from `1 560` to `1 525`.
  - Vertical alignment changed from `bottom_ref` to `top_ref`.
- Existing weather content widgets remained inside the same scroller structure.

### `GUI/layouts/vehicles/Vehicles_Menu.layout`

The vehicle manager layout was reorganized for clearer left-list and right-map/info regions.

- Root `JMVehiclesForm` style changed from `DayZDefaultPanel` to `COTSurface`.
- `vehicle_window_panel` inner list surface changed to `COTSurface`.
- `vehicles_list_label_panel` was renamed to `vehicles_list_header`.
- A dedicated `vehicles_list_buttons` panel was inserted before the scroller:
  - New size: `1 115`
  - Anchored to the bottom with `valign bottom_ref`.
  - Uses `style blank`.
- `vehicles_list_scroller` changed:
  - Position remains below the header at `0 20`.
  - Height changed from `400` to `465`.
  - Removed `halign center_ref`.
- The older `vehicles_list_buttons` block at the end of the list section was removed:
  - Old size was `1 175`.
  - The new shorter button region leaves more room for the list.
- `map_panel` no longer forces `halign right_ref`.
- Vehicle info panels were moved inside the map window panel instead of sitting as separate siblings afterward.
- New right-side information split:
  - `vehicle_info_buttons_panel`
    - Type: `FrameWidgetClass`
    - Position: `0 0`
    - Size: `0.5 1`
    - Alignment: `halign left_ref`
  - `vehicle_info_panel`
    - Type: `FrameWidgetClass`
    - Position: `0 0`
    - Size: `0.5 1`
    - Alignment: `halign right_ref`
- Removed the older separate info layout:
  - Old `vehicle_info_panel` position: `0.088 0`
  - Old `vehicle_info_panel` size: `0.45 1`
  - Old `vehicle_info_panel` alignment: `halign center_ref`
  - Old `vehicle_info_buttons_panel` size: `0.185 1`
  - Old `vehicle_info_buttons_panel` alignment: `halign right_ref`

## Detailed UIAction Layout Changes

### `GUI/layouts/uiactions/UIActionButton.layout`

Buttons gained optional icon support.

- Added `ImageWidgetClass action_icon` inside the button content.
- The icon is hidden by default with `visible 0`.
- The icon ignores pointer input.
- Position: `6 5`
- Size: `20 20`
- Exact position and exact size are enabled.
- Render mode is `blend`.
- Alpha source, no-wrap, clamp mode, and stretch mode are set for consistent `.paa` rendering.
- Existing `TextWidgetClass action` remains for the button label.

### `GUI/layouts/uiactions/UIActionNavigateButton.layout`

Navigation buttons gained the same optional icon support and a more interactive content wrapper.

- `ContentWidgetClass ContentWidget` changed to `FrameWidgetClass ContentWidget`.
- `ContentWidget` now has `ignorepointer 0` instead of `ignorepointer 1`.
- Added hidden `ImageWidgetClass action_icon`.
- Icon position: `6 5`
- Icon size: `20 20`
- Icon uses exact position/size, `blend` mode, alpha source, no-wrap, clamp, and stretch settings.
- Existing label text remains in `TextWidgetClass action`.

### `GUI/layouts/uiactions/UIActionText.layout`

Static text actions gained optional icon support.

- Added hidden `ImageWidgetClass action_icon`.
- Icon position: `6 5`
- Icon size: `20 20`
- Exact position and exact size are enabled.
- Icon rendering uses `blend`, alpha source, no-wrap, clamp, and stretch settings.
- Existing `action_label` text remains in place.

### `GUI/layouts/uiactions/UIActionCheckbox.layout`

Checkboxes were rebuilt from the stock checkbox widget into a custom skinnable row.

- `action_wrapper` changed from `GridSpacerWidgetClass` to `FrameWidgetClass`.
- Removed grid-specific settings:
  - `Columns 2`
  - `Rows 1`
  - `content_halign left`
  - `Size To Content H`
  - `Padding 0`
- Removed the old nested `checkbox_wrapper`.
- Removed the stock `CheckBoxWidgetClass action`.
- Added full-row `ButtonWidgetClass action_button`:
  - Covers the whole control with size `1 1`.
  - Uses `style Empty`.
  - Receives pointer input.
- Added square checkbox visuals:
  - `ImageWidgetClass action_box_square`
    - Hidden by default.
    - Position `4 0`
    - Size `22 22`
    - Left aligned and vertically centered.
    - Color `0.47 0.47 0.51 1`
  - `ImageWidgetClass action_check`
    - Hidden by default.
    - Position `9 0`
    - Size `12 12`
    - Color `0.2 0.6 1 1`
- Added round/radio-style visuals:
  - `ImageWidgetClass action_box_round`
    - Hidden by default.
    - Position `4 0`
    - Size `20 20`
    - Color `0.47 0.47 0.51 1`
  - `ImageWidgetClass action_check_image`
    - Hidden by default.
    - Position `9 0`
    - Size `10 10`
    - Color `0.2 0.6 1 1`
- Added `TextWidgetClass action_label` as a direct child of the frame:
  - Text now starts blank instead of placeholder `{LABEL}`.
  - Size: `1 20`
  - Left aligned and vertically centered.
  - Uses `text offset` of `28 0` so the label clears the custom checkbox icon area.
  - Keeps Metron font, bold text, exact text, and size `16`.

### `GUI/layouts/uiactions/UIActionDropdownList.layout`

The old dropdown-list edit control was converted into a more flexible search/entry field.

- Removed root `clipchildren 0`.
- `action_label` changed from always-visible label to an optional hidden label:
  - Added `visible 0`.
  - Added `ignorepointer 1`.
  - Position set to `0 0`.
  - Width changed from `1` to `0`.
  - Text changed from `{LABEL}` to blank.
  - Added explicit left alignment and exact sizing flags.
- `EditBoxWidgetClass action` changed:
  - Added `ignorepointer 0`.
  - Width changed from `0.7` to `1`.
  - Alignment changed from right-ref to left-ref.
  - Text changed from `{TEXT}` to blank.
  - Added `text offset 6 0`.
- Added hidden `EditBoxWidgetClass action_preview`:
  - Used as a non-focused preview/ghost field.
  - Hidden by default.
  - Full width with size `1 22`.
  - Grey text color `0.5 0.5 0.5 1`.
  - Uses `no focus 1`.
- Added hidden `TextWidgetClass action_placeholder`:
  - Position `10 0`
  - Size `1 22`
  - Grey text color.
  - Exact text size `14`.
- Removed the old `action_toggle_container` and nested collapse button/icon.
- Added hidden clear button:
  - `ButtonWidgetClass action_button_clear`
  - Position `-5.1 -1.1`
  - Size `24 24`
  - Right aligned and vertically centered.
  - Uses `style Empty`.
- Added clear icon inside the clear button:
  - `ImageWidgetClass action_clear_icon`
  - Position `6 6`
  - Size `12 12`
  - Color `0.8 0.8 0.8 1`
  - Uses `blend`, alpha source, no-wrap, clamp, and stretch settings.

### `GUI/layouts/uiactions/UIActionSelectBox.layout`

Select boxes were adjusted to make the label and interactive frame fill their expected space.

- `action_label` size changed from `1 30` to `1 1`.
- `action_label` now explicitly uses:
  - `halign left_ref`
  - `hexactpos 0`
  - `vexactpos 0`
  - `hexactsize 0`
  - `vexactsize 0`
- `FrameWidgetClass action` now receives pointer input:
  - `ignorepointer` changed from `1` to `0`.

### `GUI/layouts/uiactions/UIActionSlider.layout`

Sliders were rebuilt from a stock `SliderWidgetClass` into custom drawn track/fill/handle widgets.

- `action_wrapper` now receives pointer input:
  - `ignorepointer` changed from `1` to `0`.
- Removed `SliderWidgetClass action`.
- Added `PanelWidgetClass action_track`:
  - Receives pointer input.
  - Size `0.7 12`
  - Right aligned and vertically centered.
  - Color `0.06 0.06 0.1 1`
  - Style `rover_sim_colorable`.
- Added `PanelWidgetClass action_fill` inside the track:
  - Ignores pointer input.
  - Initial size `0 1`.
  - Left/top aligned.
  - Color `0.2 0.6 1 1`.
- Added `PanelWidgetClass action_handle` inside the track:
  - Receives pointer input.
  - Size `10 18`.
  - Left aligned and vertically centered.
  - Exact size enabled.
  - Color `0.5 0.8 1 1`.
- Moved/kept `TextWidgetClass action_value` inside the custom track.
- `action_value` exact text size changed from `16` to `14`.
- Added outline and shadow to `action_value`:
  - Outline size `1`
  - Outline color black
  - Shadow size `2`
  - Shadow color black
  - Shadow offset `1 1`

### `GUI/layouts/uiactions/UIPanel.layout`

- Root panel style changed from `DayZDefaultPanel` to `COTSurface`.
- Transparent color and vertical size-to-content behavior were preserved.

### `GUI/layouts/uiactions/UISpacer.layout`

- Style changed from `DayZDefaultPanel` to `Empty`.
- The spacer remains transparent and exact-height driven.

## New Top-Level Module Layouts

### `GUI/layouts/compensations_form.layout`

New compensation module form layout.

- Root: `PanelWidgetClass JMCompensationsForm`
- Script class: `JMCompensationsForm`
- Contains a primary `FrameWidgetClass panel`.
- This is a minimal form shell intended to be populated by UIAction-generated controls.

### `GUI/layouts/loot_analysis_form.layout`

New loot analysis module form layout.

- Root: `PanelWidgetClass JMLootAnalysisForm`
- Script class: `JMLootAnalysisForm`
- Defines several named panels used as anchors for generated UI:
  - `mode_panel`
  - `category_panel`
  - `loot_content_wrapper`
  - `search_panel`
  - `item_list_wrapper`
  - `item_scan_panel`
  - `item_actions_panel`
  - `loot_distribution_panel`
  - `distribution_info_panel`
  - `distribution_buttons`
- The layout separates selection/filter controls, item list content, scan controls, and distribution details into named areas.

### `GUI/layouts/territory_form.layout`

New territory module form layout.

- Root: `PanelWidgetClass JMTerritoryForm`
- Script class: `JMTerritoryForm`
- Contains a primary `FrameWidgetClass panel`.
- Like the compensation form, it provides a named form shell for script-built controls.

## New Entity Manager Layouts

### `GUI/layouts/entitymanager/EntityManager_Menu.layout`

New entity manager main layout.

- Root: `PanelWidgetClass JMEntityManagerForm`
- Script class: `JMEntityManagerForm`
- Main regions:
  - `entity_window_panel`
  - `entity_list_panel`
  - `map_window_panel`
- List area:
  - `entity_list_header`
  - `entity_list_label`
  - `entity_list_scroller`
  - `entity_list_content`
  - `entity_list_buttons`
- Map area:
  - `entity_map_panel`
  - `MapWidgetClass entity_map`
- Detail/options area:
  - `entity_options_panel`
  - `entity_info_panel`

### `GUI/layouts/entitymanager/EntityManager_List_Entry.layout`

New entity manager row layout.

- Root: `GridSpacerWidgetClass JMEntityManagerListEntry`
- Contains `entity_entry` as the visible row panel.
- Contains `edit_button` for row interaction.
- Contains `entity_status_icon` for visual state.
- Uses `entity_entry_content` as a wrapping content area.
- Displays `entity_name` and `entity_id`.
- Includes a `spacer` panel for row spacing/alignment.

### `GUI/layouts/entitymanager/EntityManager_Marker.layout`

New map marker layout for entity manager.

- Root: `FrameWidgetClass JMEntityManagerMapMarker`
- Contains `marker_panel`.
- Displays `marker_name`.
- Contains `marker_icon_panel` and `marker_icon`.
- Contains `marker_button` so the marker can be clicked/interacted with.

## New UIAction Layouts

### `GUI/layouts/uiactions/UIActionBadge.layout`

New status-badge display layout.

- Root: `PanelWidgetClass UIActionBadge`
- Script class: `UIActionBadge`
- Main wrapper: `FrameWidgetClass action_wrapper`
- Contains `action_label`.
- Contains `action_pill` for colored status presentation.
- Contains status text widget `action`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionCheckboxRow.layout`

New compact checkbox-row layout.

- Root: `PanelWidgetClass checkbox_row`
- Contains stock `CheckBoxWidgetClass action`.
- Contains `TextWidgetClass action_label`.
- Intended for list-style checkbox rows, especially inside larger generated lists.

### `GUI/layouts/uiactions/UIActionCollapsibleSection.layout`

New collapsible section layout.

- Root: `PanelWidgetClass UIActionCollapsibleSection`
- Script class: `UIActionCollapsibleSection`
- Uses `GridSpacerWidgetClass action_wrapper`.
- Header click target: `ButtonWidgetClass action_button`.
- Visual elements:
  - `fill`
  - `border_bottom`
  - `action_label`
  - `action_arrow`
- Expandable body anchor: `PanelWidgetClass action_content`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionColorPicker.layout`

New color picker layout.

- Root: `PanelWidgetClass UIActionColorPicker`
- Script class: `UIActionColorPicker`
- Uses `FrameWidgetClass action_wrapper`.
- Contains `action_label`.
- Contains `action_preview` color swatch panel.
- Contains `EditBoxWidgetClass action` for typed color input.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionConfirmInline.layout`

New inline confirmation button layout.

- Root: `PanelWidgetClass UIActionConfirmInline`
- Script class: `UIActionConfirmInline`
- Main button:
  - `action_button`
  - `fill`
  - `border_top`
  - `border_bottom`
  - optional `action_icon`
  - main text widget `action`
- Confirmation controls:
  - `action_button_confirm`
  - `confirm_text`
  - `action_button_cancel`
  - `cancel_text`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionDataTable.layout`

New data table layout.

- Root: `PanelWidgetClass UIActionDataTable`
- Script class: `UIActionDataTable`
- Uses `FrameWidgetClass action_wrapper`.
- Header container: `action_header_container`.
- Scroll region: `ScrollWidgetClass action_scroll`.
- Body container: `action_body_container`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionDropdown.layout`

New styled dropdown layout.

- Root: `PanelWidgetClass UIActionDropdown`
- Script class: `UIActionDropdown`
- Uses `FrameWidgetClass action_wrapper`.
- Contains:
  - `action_label`
  - `action_field`
  - `action_selected_icon`
  - `action_selected_text`
  - `action_toggle`
  - `action_toggle_image`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionDropdown_List.layout`

New dropdown list container.

- Root: `PanelWidgetClass UIActionDropdown_List`
- Contains `GridSpacerWidgetClass action_list_grid`.
- Used as the popup/list body for the new dropdown control.

### `GUI/layouts/uiactions/UIActionDropdown_Row.layout`

New dropdown list row.

- Root: `ButtonWidgetClass UIActionDropdownRow`
- Contains `ImageWidgetClass row_icon`.
- Contains `TextWidgetClass row_text`.
- Supports icon-plus-label dropdown entries.

### `GUI/layouts/uiactions/UIActionFilterBar.layout`

New filter bar layout.

- Root: `PanelWidgetClass UIActionFilterBar`
- Script class: `UIActionFilterBar`
- Main wrapper: `FrameWidgetClass action_wrapper`.
- Contains standard disabled overlay `action_wrapper_disable`.
- Individual buttons are provided by `UIActionFilterButton.layout`.

### `GUI/layouts/uiactions/UIActionFilterButton.layout`

New filter button layout.

- Root: `ButtonWidgetClass filter_btn`
- Contains visual fill panel `fill`.
- Contains text widget `label`.

### `GUI/layouts/uiactions/UIActionIconCell.layout`

New icon grid cell layout.

- Root: `PanelWidgetClass icon_cell`
- Contains `ButtonWidgetClass action_button`.
- Contains visual fill panel `fill`.
- Contains `ImageWidgetClass action_image`.

### `GUI/layouts/uiactions/UIActionIconGrid.layout`

New icon grid container layout.

- Root: `GridSpacerWidgetClass UIActionIconGrid`
- Script class: `UIActionIconGrid`
- Contains standard disabled overlay `action_wrapper_disable`.
- Individual cells are provided by `UIActionIconCell.layout`.

### `GUI/layouts/uiactions/UIActionKeyValueList.layout`

New key-value list layout.

- Root: `PanelWidgetClass UIActionKeyValueList`
- Script class: `UIActionKeyValueList`
- Contains `GridSpacerWidgetClass action_rows`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionLogLine.layout`

New log line layout.

- Root: `TextWidgetClass log_line`
- Used as a single row inside log-view style controls.

### `GUI/layouts/uiactions/UIActionLogView.layout`

New scrollable log view layout.

- Root: `PanelWidgetClass UIActionLogView`
- Script class: `UIActionLogView`
- Uses `FrameWidgetClass action_wrapper`.
- Contains `action_background`.
- Contains `ScrollWidgetClass action_scroll`.
- Contains `GridSpacerWidgetClass action_lines`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionMultiSelectList.layout`

New multi-select list layout.

- Root: `PanelWidgetClass UIActionMultiSelectList`
- Script class: `UIActionMultiSelectList`
- Uses `FrameWidgetClass action_wrapper`.
- Contains:
  - `action_rows_outer`
  - `action_label`
  - `action_list_container`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionPaginator.layout`

New paginator layout.

- Root: `PanelWidgetClass UIActionPaginator`
- Script class: `UIActionPaginator`
- Contains previous button:
  - `action_button_prev`
  - `prev_text`
- Contains center page text widget `action`.
- Contains next button:
  - `action_button_next`
  - `next_text`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionProgressBar.layout`

New progress bar layout.

- Root: `PanelWidgetClass UIActionProgressBar`
- Script class: `UIActionProgressBar`
- Contains:
  - `action_label`
  - `action_track`
  - `action_fill`
  - `action` text value
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionSearchBox.layout`

New search box layout.

- Root: `PanelWidgetClass UIActionSearchBox`
- Script class: `UIActionSearchBox`
- Contains:
  - `action_label`
  - main `EditBoxWidgetClass action`
  - hidden/preview `EditBoxWidgetClass action_preview`
  - `action_placeholder`
  - `action_button_clear`
  - `action_clear_icon`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionSliderRange.layout`

New dual-handle range slider layout.

- Root: `PanelWidgetClass UIActionSliderRange`
- Script class: `UIActionSliderRange`
- Contains:
  - `action_label`
  - `action` value text
  - `action_track`
  - `action_fill`
  - `action_handle_low`
  - `action_handle_high`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionSpinner.layout`

New numeric spinner layout.

- Root: `PanelWidgetClass UIActionSpinner`
- Script class: `UIActionSpinner`
- Contains:
  - `action_label`
  - `action_controls`
  - `EditBoxWidgetClass action`
  - decrement button `action_button_dec`
  - increment button `action_button_inc`
  - `dec_text`
  - `inc_text`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionStagedIcon.layout`

New staged icon control layout.

- Root: `PanelWidgetClass UIActionStagedIcon`
- Script class: `UIActionStagedIcon`
- Contains `ButtonWidgetClass action_button`.
- Contains `ImageWidgetClass action_image`.
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionStepList.layout`

New ordered step list layout.

- Root: `PanelWidgetClass UIActionStepList`
- Script class: `UIActionStepList`
- Contains:
  - `action_label`
  - `GridSpacerWidgetClass action_rows`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionTabs.layout`

New tabs layout.

- Root: `PanelWidgetClass UIActionTabs`
- Script class: `UIActionTabs`
- Uses `FrameWidgetClass action_wrapper`.
- Contains standard disabled overlay `action_wrapper_disable`.
- Tab button rows are provided by the `Wrappers/TabBar/` layouts.

### `GUI/layouts/uiactions/UIActionTimePicker.layout`

New time picker layout.

- Root: `PanelWidgetClass UIActionTimePicker`
- Script class: `UIActionTimePicker`
- Contains:
  - `action_label`
  - `GridSpacerWidgetClass action_fields`
  - `EditBoxWidgetClass action_hours`
  - `EditBoxWidgetClass action_minutes`
  - `EditBoxWidgetClass action_seconds`
  - separators `action_sep1` and `action_sep2`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionToggle.layout`

New custom toggle layout.

- Root: `PanelWidgetClass UIActionToggle`
- Script class: `UIActionToggle`
- Uses the same custom visual pieces as the rebuilt checkbox:
  - `action_button`
  - `action_box_square`
  - `action_check`
  - `action_box_round`
  - `action_check_image`
  - `action_label`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionToggleSwitch.layout`

New switch-style toggle layout.

- Root: `PanelWidgetClass UIActionToggleSwitch`
- Script class: `UIActionToggleSwitch`
- Contains:
  - `action_label`
  - `action_button`
  - `action_track`
  - `action_thumb`
- Contains standard disabled overlay `action_wrapper_disable`.

### `GUI/layouts/uiactions/UIActionTooltip.layout`

New tooltip overlay layout.

- Root: `PanelWidgetClass UIActionTooltip`
- Contains:
  - `border`
  - `tooltip_swatch`
  - `tooltip_icon`
  - `tooltip_text_container`
- Used as the shared hover tooltip surface for UI actions.

## New Wrapper Layouts

Additional generated wrapper layouts were added for denser generated UI rows.

### Grid wrappers

New folders:

- `GUI/layouts/uiactions/Wrappers/10/`
- `GUI/layouts/uiactions/Wrappers/11/`
- `GUI/layouts/uiactions/Wrappers/12/`
- `GUI/layouts/uiactions/Wrappers/13/`
- `GUI/layouts/uiactions/Wrappers/14/`
- `GUI/layouts/uiactions/Wrappers/15/`
- `GUI/layouts/uiactions/Wrappers/16/`
- `GUI/layouts/uiactions/Wrappers/17/`
- `GUI/layouts/uiactions/Wrappers/18/`
- `GUI/layouts/uiactions/Wrappers/19/`
- `GUI/layouts/uiactions/Wrappers/20/`

Each folder contains `GridSpacer1.layout` through `GridSpacer9.layout`.

- Root widget: `GridSpacerWidgetClass spacer`
- Purpose: provide reusable grid wrappers for dynamically created UI sections with larger row/column counts than the previous wrapper set supported.

### Tab bar wrappers

New folder:

- `GUI/layouts/uiactions/Wrappers/TabBar/`

Files:

- `TabBar1.layout`
- `TabBar2.layout`
- `TabBar3.layout`
- `TabBar4.layout`
- `TabBar5.layout`
- `TabBar6.layout`
- `TabBar7.layout`
- `TabBar8.layout`
- `TabBar9.layout`

Each tab-bar wrapper uses:

- Root widget: `GridSpacerWidgetClass tab_bar`
- Purpose: provide fixed-count tab row layouts for `UIActionTabs`.

## Renamed Layout

### `GUI/layouts/sidebar_button.Layout` -> `GUI/layouts/sidebar_button.layout`

- The file was renamed to use a lowercase `.layout` extension.
- This makes the filename consistent with the rest of the layout folder.
- No content change was reported for the rename itself.

## Removed Layouts

### `GUI/layouts/camera_form_PHI.layout`

Removed old hidden `CameraPHI` reticle/grid overlay layout.

- Root was `PanelWidgetClass CameraPHI`.
- It was hidden by default with `visible 0`.
- It contained four white editor-style guide lines:
  - `line_horizontal_bottom`
  - `line_horizontal_top`
  - `line_vertical_left`
  - `line_vertical_right`
- The guide lines used the `0.618` placement ratio.

### `GUI/layouts/camera_form_ROT.layout`

Removed old hidden `CameraROT` reticle/grid overlay layout.

- Root was `PanelWidgetClass CameraROT`.
- It was hidden by default with `visible 0`.
- It contained four white editor-style guide lines:
  - `line_horizontal_top`
  - `line_horizontal_bottom`
  - `line_vertical_left`
  - `line_vertical_right`
- The guide lines used the `0.3333` placement ratio.

### `GUI/layouts/debug_form.layout`

Removed old debug form layout.

- Root was `PanelWidgetClass JMDebugForm`.
- Fixed size was `250 420`.
- Script class was `JMDebugForm`.
- It contained:
  - `ScrollWidgetClass actions_scroller`
  - `GridSpacerWidgetClass actions_wrapper`
- The wrapper used one column and up to `1000` rows for generated debug actions.

### `GUI/layouts/weather_form_old.layout`

Removed old weather form layout.

- Root was `PanelWidgetClass JMWeatherOldForm`.
- Fixed size was `570 295`.
- Script class was `JMWeatherOldForm`.
- It used `DayZDefaultPanel`.
- It contained a large hand-authored `slider_wrapper` with many explicit weather slider rows.
- It represented the older slider-heavy weather UI and was superseded by the current `weather_form.layout` plus reusable `UIAction` controls.

## Related Non-Layout Support

These are not `.layout` files, but they exist to support the layout changes above.

- `GUI/styles/cot.styles`
  - Adds `COTSurface`.
- `GUI/textures/icons/*.paa`
  - New and updated icons consumed by window chrome, buttons, dropdowns, status controls, weather controls, vehicle controls, and new UIAction layouts.
- `Scripts/5_Mission/CommunityOnlineTools/gui/Actions/UIActionManager.c`
  - Adds creation helpers for the new `UIAction` layouts.
- `Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMWindowBase.c`
  - Reads the new image widgets in `windowbase.layout`.
- `Scripts/4_World/CommunityOnlineTools/Classes/GUI/JMUIConstants.c`
  - Centralizes colors, layout widths, and animation constants referenced by the updated UI code.

## Practical Result

The layout layer is now more consistent and more reusable. Existing forms use the same `COTSurface` treatment, shared window controls use COT-owned image assets, and the expanded UIAction layout set gives new screens a much larger library of search, filtering, table, dropdown, badge, progress, tab, tooltip, and picker controls.

## Form-Level Restructures (2026-07-06)

Several module forms were restructured to fix row-layout bugs discovered in
shipping playtests. The unifying principle: **toolbar rows and per-entry
rows are inline horizontal flows, never a 1×N grid split**. The grid split
made the icon-button sit in a wide empty cell next to its sibling and
caused labels to truncate at narrow panel widths.

### `webhook_form.layout`

- Root size changed from `540 480` (fixed pixel) to `1 1` (fractional).
- `hexactsize` / `vexactsize` flipped to `0` so the form fills its parent
  sidebar instead of leaving empty space below.
- `header_panel` unchanged: still 35 px tall, full width.
- `panel` size changed from `1 445` (fixed pixel height) to `1 -35`
  (relative offset from header). `vexactsize` flipped to `0` so the
  scroller fills the remaining height.
- Rationale: hosted in a sidebar whose height exceeds 480 px; the
  previous fixed pixel size left visible whitespace below the form.

### `mapeditor_form.layout`

The map editor's three top-level panels (`panel_top`, `panel_left`,
`panel_right`) kept their sizing rules, but the script-side build was
reworked:

- `JMMapEditorForm.OnInit()` switched the top toolbar's mode-row and
  snap-row from `GridSpacer(1, N)` (N buttons forced into one row at
  fixed cell widths) to `WrapSpacer` (flex-wrap flow). When the panel
  is narrower than the button row, buttons wrap to a new line instead
  of being truncated by fixed cells.
- The 3D viewport IS the full centre area between the sidebars. No
  widget fills it — the JMFreecam player view IS the viewport, and the
  form's `Tick()` draws the gizmo + selection wireframe directly with
  `Debug.DrawLine` / `Debug.DrawArrow`.

### `JMWebhookSection` (header row restructure)

The header for each webhook section was restructured from
`GridSpacer`-stacked (one coloured panel + name text on its own row,
then Save/Remove on a second row) to a single inline `WrapSpacer` with
`[name] ──────── [Remove]` on one line, followed by Save on the next
row as full-width green.

```c
// Header row: text + spacer + Remove on the same line.
Widget headerRow = UIActionManager.CreateWrapSpacer( m_RootSpacer,
    WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );
UIActionText nameTxt = UIActionManager.CreateText( headerRow,
    "[ " + group.Name + " ]" );
nameTxt.SetWidth( 1.0 - 0.12 );

UIActionConfirmInline removeBtn = UIActionManager.CreateConfirmInline(
    headerRow, "Remove", m_Form, "Action_RemoveWebhook" );
UIActionIconGrid.ApplyDeletePreset( removeBtn );
removeBtn.SetFixedSize( 32, 32 );
removeBtn.SetData( new JMWebhookTypeData( group.Name ) );
removeBtn.SetTooltip( "Remove this webhook" );
```

Save stays as its own full-width row below.

### `JMBanForm` (`JMPlayerBanForm.c`)

- `OnManualBan_GotSteamID()` no longer calls the reason-prompt
  `CreateConfirmation_Two` synchronously. The second popup is deferred
  by one `CallLater` tick (`OpenBanReasonPopup`, ~50 ms) so the engine
  has time to release the first modal's focus lock before the second
  one tries to acquire it. Chained `JMConfirmationType.EDIT` modals
  were silently failing to open the second popup; this workaround
  resolves it.
- The deferral also guards the case where `confirmation.GetEditBoxValue()`
  returns empty — the deferred call checks `m_PendingSteamID == ""` and
  bails before showing the reason prompt.

### `JMCompensationsForm` (toolbar + per-entry row)

Toolbar row:

- Switched from `GridSpacer(1, 2)` (refresh icon sat in a wide half-row
  cell, dropdown sat in the other half) to `WrapSpacer` with
  `refresh (32×32 fixed) + dropdown (0.85)`. Refresh is now a tight
  square; dropdown fills the remaining row.

Per-entry row:

- Order is `[Delete icon] [Spawn] [Timestamp]` with optional inline
  player-name label on the first entry of each player section.
- Delete is icon-only (`SetButton("")` + `CenterIcon(32, 16)`) at
  32×32 fixed. Compact confirm labels (`SetConfirmLabel("O")`,
  `SetCancelLabel("X")`) so the confirm gesture fits inside the 32 px
  square — full "Confirm"/"Cancel" clips at narrow widths.
- Width budgets per row:
  - First entry per player: `Spawn(0.18) + Label(0.35) + Timestamp(0.55)` = 0.93.
  - Subsequent entries: `Spawn(0.18) + Timestamp(0.78)` = 0.96.
- `OnClick_Spawn()` now coerces `GetSelection() == -1` to the default
  mode and has a `default:` case that shows a popup instead of silently
  no-oping on unrecognized modes.

### `JMLoadoutForm` (mirror of compensations)

- Toolbar matches the compensations toolbar: refresh (32×32) + dropdown
  (0.85) on one `WrapSpacer` row.
- Per-entry row: `[Delete icon] [Spawn] [Loadout name]` — same icon-only
  delete, compact "O"/"X" confirm labels, `Spawn(0.20) + Name(0.74)`.
- `OnClick_Spawn()` applies the same `-1 → CURSOR` coercion and
  `default:`-case popup guard as compensations.

### `JMVehiclesForm` (bulk-delete row)

- The three bulk-delete actions (`Delete All`, `Delete Destroyed`,
  `Delete Unclaimed`) were moved from `GridSpacer(1, 3)` (three
  buttons sharing one row, labels truncated at narrow widths) to
  `GridSpacer(3, 1)` (one button per row, full width).
- Mirrors the entity manager layout in `JMEntityManagerForm.c`, where
  each bulk action gets its own row at full width.

### `JMObjectSpawnerForm` (footer refresh)

- The refresh icon was moved from the search row down into the
  `UIActionFlexRow` footer (where Delete + Mode + Setup + Spawn
  already live). Now the footer is Refresh + Delete + Mode + Setup +
  Spawn on one line.

## Summary of New UI Patterns

Three patterns emerged from this round and apply across all module
forms:

### Pattern: Refresh icon + dropdown toolbar

```c
Widget toolbar = UIActionManager.CreateWrapSpacer( parent,
    WidgetAlignment.WA_LEFT, WidgetAlignment.WA_CENTER );

UIActionImageButton refresh = UIActionManager.CreateIconButton(
    toolbar, JMConstants.ICON_CLOCKWISE, this, "OnClick_Refresh" );
refresh.SetFixedSize( 32, 32 );

UIActionDropdown dropdown = UIActionManager.CreateDropdown(
    toolbar, "", layoutRoot, this, "OnClick_SpawnMode", options );
dropdown.SetWidth( 0.85 );   // sum with refresh-px leaves margin
```

### Pattern: Icon-only ConfirmInline in a per-entry row

```c
UIActionConfirmInline delbttn = UIActionManager.CreateConfirmInline(
    row, "", this, "OnClick_Delete" );
UIActionIconGrid.ApplyDeletePreset( delbttn );
delbttn.SetButton( "" );
delbttn.SetFixedSize( 32, 32 );
delbttn.CenterIcon( 32, 16 );
delbttn.SetConfirmLabel( "O" );
delbttn.SetCancelLabel( "X" );
```

Compact confirm labels are the difference between "Confirm"/"Cancel"
fitting and clipping at narrow widths.

### Pattern: Stacked bulk-action rows (entity-manager style)

```c
Widget bulkRow = UIActionManager.CreateGridSpacer( parent, N, 1 );
foreach ( action : bulkActions )
{
    UIActionConfirmInline b = UIActionManager.CreateConfirmInline(
        bulkRow, action.m_Label, this, "OnBulkClick" );
    UIActionIconGrid.ApplyDeletePreset( b );
}
```

Each action gets its own row at full width. Labels stay legible on
every panel width.

## Engine Quirk: Chained CreateConfirmation_Two

Calling `CreateConfirmation_Two` from inside the callback of another
`CreateConfirmation_Two` (e.g. SteamID prompt → reason prompt) was
silently failing to open the second popup. Workaround: defer the
second call by one `CallLater` tick (`CALL_CATEGORY_GUI`, ~50 ms) so
the first modal fully tears down before the second acquires its
focus lock. See `JMPlayerBanForm.OpenBanReasonPopup` for the
canonical implementation.
