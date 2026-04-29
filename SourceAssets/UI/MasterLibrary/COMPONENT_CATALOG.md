# MasterLibrary Component Catalog

Style pass: `solid_purple_two_ring_pass_20260428`

Source concept: `SourceAssets/UI/MasterLibrary/Concepts/20260428_basic_button_panel_pass02/basic_button_panel_concept_board_pass02_approved_style.png`

## Visual Rules

- Solid dark navy fill, visibly blue rather than black.
- Two solid purple border rings: bright outer ring plus darker inner ring.
- No labels, icons, scanlines, dots, greebles, orange accent marks, or baked content.
- Transparent outside pixels for runtime-safe Slate box brush use.
- Shared UI text should use the off-white `FT66Style::Text()` color rather than pure white.

## Buttons

| Family | Files | Dimensions | 9-slice margin | Use |
| --- | --- | --- | --- | --- |
| `basic_button` | `normal`, `hover`, `pressed`, `disabled` | `270x88` | `0.104, 0.250, 0.104, 0.250` | Neutral/shared action buttons |
| `central_button` | `normal`, `hover`, `pressed`, `disabled` | `360x104` | `0.083, 0.231, 0.083, 0.231` | Main-menu CTA buttons |
| `select_button` | `normal`, `hover`, `pressed`, `selected`, `disabled` | `270x88` | `0.104, 0.250, 0.104, 0.250` | Tabs and selectable actions |
| `duo_button_left` | `normal`, `hover`, `pressed`, `selected`, `disabled` | `270x88` | `0.104, 0.250, 0.027, 0.250` | Left half of paired controls |
| `duo_button_right` | `normal`, `hover`, `pressed`, `selected`, `disabled` | `270x88` | `0.027, 0.250, 0.104, 0.250` | Right half of paired controls |
| `dropdown_option_button` | `normal`, `hover`, `pressed`, `disabled` | `420x56` | `0.067, 0.250, 0.067, 0.250` | Dropdown option plates |
| `borderless_icon_button` | `normal`, `hover`, `pressed`, `selected`, `disabled` | `96x96` | `0, 0, 0, 0` | Icon hit targets; normal remains transparent |

## Panels

| Family | File | Dimensions | 9-slice margin | Use |
| --- | --- | --- | --- | --- |
| `basic_panel` | `basic_panel_normal.png` | `480x800` | `0.067, 0.043, 0.067, 0.043` | Primary content panels and modal shells |
| `inner_panel` | `inner_panel_normal.png` | `620x340` | `0.067, 0.043, 0.067, 0.043` | Secondary/nested content surfaces |

Machine-readable catalog: `SourceAssets/UI/MasterLibrary/component_catalog.json`
