# UT66 Cowardice Prompt Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66CowardicePromptWidget\baseline_dump.json`

## Structure

- [ ] CowardicePrompt.Root | exists=true
- [ ] CowardicePrompt.Root | type=SOverlay
- [ ] CowardicePrompt.Panel | exists=true
- [ ] CowardicePrompt.Panel | type=SBorder
- [ ] CowardicePrompt.Title | exists=true
- [ ] CowardicePrompt.Title | type=STextBlock
- [ ] CowardicePrompt.Status | exists=true
- [ ] CowardicePrompt.Status | type=STextBlock
- [ ] CowardicePrompt.Actions | exists=true
- [ ] CowardicePrompt.Actions | type=SHorizontalBox
- [ ] CowardicePrompt.YesButton | exists=true
- [ ] CowardicePrompt.YesButton | type=SBox
- [ ] CowardicePrompt.NoButton | exists=true
- [ ] CowardicePrompt.NoButton | type=SBox

## Geometry

- [ ] CowardicePrompt.Root | x=0.0000 | 0.005
- [ ] CowardicePrompt.Root | y=0.0000 | 0.005
- [ ] CowardicePrompt.Root | w=1.0000 | 0.005
- [ ] CowardicePrompt.Root | h=1.0000 | 0.005
- [ ] CowardicePrompt.Panel | x=0.3850 | 0.020
- [ ] CowardicePrompt.Panel | y=0.4113 | 0.020
- [ ] CowardicePrompt.Panel | w=0.2300 | 0.020
- [ ] CowardicePrompt.Panel | h=0.1774 | 0.020
- [ ] CowardicePrompt.Title | x=0.4294 | 0.020
- [ ] CowardicePrompt.Title | y=0.4380 | 0.020
- [ ] CowardicePrompt.Title | w=0.1411 | 0.020
- [ ] CowardicePrompt.Title | h=0.0352 | 0.020
- [ ] CowardicePrompt.Status | x=0.4000 | 0.020
- [ ] CowardicePrompt.Status | y=0.4843 | 0.020
- [ ] CowardicePrompt.Status | w=0.2000 | 0.020
- [ ] CowardicePrompt.Status | h=0.0267 | 0.020
- [ ] CowardicePrompt.Actions | x=0.4000 | 0.020
- [ ] CowardicePrompt.Actions | y=0.5243 | 0.020
- [ ] CowardicePrompt.Actions | w=0.2000 | 0.020
- [ ] CowardicePrompt.Actions | h=0.0378 | 0.020
- [ ] CowardicePrompt.YesButton | x=0.4063 | 0.020
- [ ] CowardicePrompt.YesButton | y=0.5243 | 0.020
- [ ] CowardicePrompt.YesButton | w=0.0875 | 0.020
- [ ] CowardicePrompt.YesButton | h=0.0378 | 0.020
- [ ] CowardicePrompt.NoButton | x=0.5063 | 0.020
- [ ] CowardicePrompt.NoButton | y=0.5243 | 0.020
- [ ] CowardicePrompt.NoButton | w=0.0875 | 0.020
- [ ] CowardicePrompt.NoButton | h=0.0378 | 0.020

## Colors

- [ ] CowardicePrompt.Panel | border_color=DefaultBorder
- [ ] CowardicePrompt.Panel | state=Default
- [ ] CowardicePrompt.Title | text_color=PrimaryText
- [ ] CowardicePrompt.Status | text_color=SecondaryText
- [ ] CowardicePrompt.YesButton | border_color=SelectedBorder
- [ ] CowardicePrompt.YesButton | state=Selected
- [ ] CowardicePrompt.NoButton | border_color=DefaultBorder
- [ ] CowardicePrompt.NoButton | state=Default

## Content

- [ ] CowardicePrompt.Root | role=Overlay
- [ ] CowardicePrompt.Panel | role=Panel
- [ ] CowardicePrompt.Title | role=Label.Header
- [ ] CowardicePrompt.Title | text=Take Cowardice Gate?
- [ ] CowardicePrompt.Title | is_label=true
- [ ] CowardicePrompt.Status | role=Label.Body
- [ ] CowardicePrompt.Status | is_label=true
- [ ] CowardicePrompt.Actions | role=ActionRow
- [ ] CowardicePrompt.YesButton | role=Button
- [ ] CowardicePrompt.YesButton | text=YES
- [ ] CowardicePrompt.NoButton | role=Button
- [ ] CowardicePrompt.NoButton | text=NO

## Interactivity

- [ ] CowardicePrompt.Root | has_click_handler=false
- [ ] CowardicePrompt.Panel | has_click_handler=false
- [ ] CowardicePrompt.Title | has_click_handler=false
- [ ] CowardicePrompt.Title | hover_capable=false
- [ ] CowardicePrompt.Status | has_click_handler=false
- [ ] CowardicePrompt.Status | hover_capable=false
- [ ] CowardicePrompt.YesButton | has_click_handler=true
- [ ] CowardicePrompt.YesButton | hover_capable=true
- [ ] CowardicePrompt.NoButton | has_click_handler=true
- [ ] CowardicePrompt.NoButton | hover_capable=true
