# ST66 World Map Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\ST66WorldMapWidget\baseline_dump.json`

## Structure

- [ ] GameplayHUD.Minimap.Frame | exists=true
- [ ] GameplayHUD.Minimap.Frame | type=SBorder
- [ ] GameplayHUD.Minimap.Map | exists=true
- [ ] GameplayHUD.Minimap.Map | type=ST66WorldMapWidget

## Geometry

- [ ] GameplayHUD.Minimap.Frame | x=0.0000 | 0.005
- [ ] GameplayHUD.Minimap.Frame | y=0.0000 | 0.005
- [ ] GameplayHUD.Minimap.Frame | w=0.1230 | 0.005
- [ ] GameplayHUD.Minimap.Frame | h=0.2187 | 0.005
- [ ] GameplayHUD.Minimap.Map | x=0.0000 | 0.020
- [ ] GameplayHUD.Minimap.Map | y=0.0000 | 0.020
- [ ] GameplayHUD.Minimap.Map | w=0.1110 | 0.020
- [ ] GameplayHUD.Minimap.Map | h=0.1973 | 0.020
- [ ] GameplayHUD.Minimap.Map | absolute_width=213.120 | 12.000
- [ ] GameplayHUD.Minimap.Map | absolute_height=213.120 | 12.000

## Colors

- [ ] GameplayHUD.Minimap.Frame | border_color=DefaultBorder
- [ ] GameplayHUD.Minimap.Frame | state=Default
- [ ] GameplayHUD.Minimap.Map | state=Default

## Content

- [ ] GameplayHUD.Minimap.Frame | role=Panel
- [ ] GameplayHUD.Minimap.Frame | is_label=false
- [ ] GameplayHUD.Minimap.Map | role=MapContent
- [ ] GameplayHUD.Minimap.Map | is_label=false

## Interactivity

- [ ] GameplayHUD.Minimap.Frame | has_click_handler=false
- [ ] GameplayHUD.Minimap.Frame | hover_capable=false
- [ ] GameplayHUD.Minimap.Map | has_click_handler=false
- [ ] GameplayHUD.Minimap.Map | hover_capable=false
