# UT66 Gameplay HUD Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66GameplayHUDWidget\baseline_dump.json`

## Structure

- [ ] Widget.Class.UT66GameplayHUDWidget.Root | exists=true
- [ ] Widget.Class.UT66GameplayHUDWidget.Root | type=SObjectWidget
- [ ] GameplayHUD.LevelRing | exists=true
- [ ] GameplayHUD.LevelRing | type=ST66RingWidget
- [ ] GameplayHUD.Minimap.Frame | exists=true
- [ ] GameplayHUD.Minimap.Frame | type=SBorder
- [ ] GameplayHUD.Minimap.Map | exists=true
- [ ] GameplayHUD.Minimap.Map | type=ST66WorldMapWidget
- [ ] WorldInteractablePrompt.Root | exists=true
- [ ] WorldInteractablePrompt.Panel | exists=true
- [ ] WorldInteractablePrompt.Text | exists=true
- [ ] GameplayHUD.Crosshair | exists=true
- [ ] GameplayHUD.Crosshair | type=ST66CrosshairWidget

## Geometry

- [ ] Widget.Class.UT66GameplayHUDWidget.Root | x=0.0000 | 0.005
- [ ] Widget.Class.UT66GameplayHUDWidget.Root | y=0.0000 | 0.005
- [ ] Widget.Class.UT66GameplayHUDWidget.Root | w=1.0000 | 0.005
- [ ] Widget.Class.UT66GameplayHUDWidget.Root | h=1.0000 | 0.005
- [ ] GameplayHUD.LevelRing | x=0.1445 | 0.010
- [ ] GameplayHUD.LevelRing | y=0.8459 | 0.010
- [ ] GameplayHUD.LevelRing | w=0.0221 | 0.010
- [ ] GameplayHUD.LevelRing | h=0.0392 | 0.010
- [ ] GameplayHUD.Minimap.Frame | x=0.8680 | 0.010
- [ ] GameplayHUD.Minimap.Frame | y=0.0160 | 0.010
- [ ] GameplayHUD.Minimap.Frame | w=0.1230 | 0.010
- [ ] GameplayHUD.Minimap.Frame | h=0.2187 | 0.010
- [ ] GameplayHUD.Minimap.Map | x=0.8755 | 0.020
- [ ] GameplayHUD.Minimap.Map | y=0.0293 | 0.020
- [ ] GameplayHUD.Minimap.Map | w=0.1080 | 0.020
- [ ] GameplayHUD.Minimap.Map | h=0.1920 | 0.020
- [ ] WorldInteractablePrompt.Root | x=0.3350 | 0.020
- [ ] WorldInteractablePrompt.Root | y=0.9120 | 0.020
- [ ] WorldInteractablePrompt.Root | w=0.3300 | 0.020
- [ ] WorldInteractablePrompt.Root | h=0.0720 | 0.020
- [ ] GameplayHUD.Crosshair | x=0.4895 | 0.010
- [ ] GameplayHUD.Crosshair | y=0.3085 | 0.010
- [ ] GameplayHUD.Crosshair | w=0.0210 | 0.010
- [ ] GameplayHUD.Crosshair | h=0.0373 | 0.010

## Colors

- [ ] GameplayHUD.LevelRing | border_color=DefaultBorder
- [ ] GameplayHUD.Minimap.Frame | border_color=DefaultBorder
- [ ] WorldInteractablePrompt.Root | border_color=DefaultBorder
- [ ] WorldInteractablePrompt.Panel | border_color=DefaultBorder
- [ ] WorldInteractablePrompt.Text | text_color=PrimaryText
- [ ] GameplayHUD.Crosshair | border_color=PrimaryText
- [ ] GameplayHUD.LevelRing | state=Default
- [ ] GameplayHUD.Minimap.Frame | state=Default
- [ ] WorldInteractablePrompt.Root | state=Default
- [ ] GameplayHUD.Crosshair | state=Default

## Content

- [ ] GameplayHUD.LevelRing | role=HUDChromeRing
- [ ] GameplayHUD.Minimap.Frame | role=Panel
- [ ] GameplayHUD.Minimap.Map | role=MapContent
- [ ] WorldInteractablePrompt.Root | role=InWorldPrompt
- [ ] WorldInteractablePrompt.Panel | role=Panel
- [ ] WorldInteractablePrompt.Text | role=Label.Body
- [ ] GameplayHUD.Crosshair | role=GameplayReticle
- [ ] WorldInteractablePrompt.Text | is_label=true
- [ ] GameplayHUD.Minimap.Map | is_label=false
- [ ] GameplayHUD.Crosshair | is_label=false

## Interactivity

- [ ] GameplayHUD.LevelRing | has_click_handler=false
- [ ] GameplayHUD.LevelRing | hover_capable=false
- [ ] GameplayHUD.Minimap.Frame | has_click_handler=false
- [ ] GameplayHUD.Minimap.Frame | hover_capable=false
- [ ] GameplayHUD.Minimap.Map | has_click_handler=false
- [ ] GameplayHUD.Minimap.Map | hover_capable=false
- [ ] WorldInteractablePrompt.Root | has_click_handler=false
- [ ] WorldInteractablePrompt.Root | hover_capable=false
- [ ] GameplayHUD.Crosshair | has_click_handler=false
- [ ] GameplayHUD.Crosshair | hover_capable=false
