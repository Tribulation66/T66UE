# UT66 Loading Screen Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66LoadingScreenWidget\baseline_dump.json`

## Structure

- [ ] LoadingScreen.Background | exists=true
- [ ] LoadingScreen.Background | type=SBorder
- [ ] LoadingScreen.Text | exists=true
- [ ] LoadingScreen.Text | type=SBox

## Geometry

- [ ] LoadingScreen.Background | x=0.0000 | 0.005
- [ ] LoadingScreen.Background | y=0.0000 | 0.005
- [ ] LoadingScreen.Background | w=1.0000 | 0.005
- [ ] LoadingScreen.Background | h=1.0000 | 0.005
- [ ] LoadingScreen.Text | x=0.4193 | 0.005
- [ ] LoadingScreen.Text | y=0.4556 | 0.005
- [ ] LoadingScreen.Text | w=0.1615 | 0.005
- [ ] LoadingScreen.Text | h=0.0889 | 0.005
- [ ] LoadingScreen.Text | absolute_width=310.000 | 4.000
- [ ] LoadingScreen.Text | absolute_height=96.000 | 4.000

## Colors

- [ ] LoadingScreen.Background | border_color=BackgroundColor
- [ ] LoadingScreen.Background | state=Default
- [ ] LoadingScreen.Text | text_color=PrimaryText

## Content

- [ ] LoadingScreen.Background | role=FullscreenOverlay
- [ ] LoadingScreen.Text | role=Label.Title
- [ ] LoadingScreen.Text | text=Loading
- [ ] LoadingScreen.Text | font_size=56
- [ ] LoadingScreen.Background | is_label=false
- [ ] LoadingScreen.Text | is_label=true

## Interactivity

- [ ] LoadingScreen.Background | has_click_handler=false
- [ ] LoadingScreen.Background | hover_capable=false
- [ ] LoadingScreen.Text | has_click_handler=false
- [ ] LoadingScreen.Text | hover_capable=false
