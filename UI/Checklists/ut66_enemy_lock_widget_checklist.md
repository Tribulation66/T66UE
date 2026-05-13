# UT66 Enemy Lock Widget Structural Preservation Checklist

Source baseline dump: `C:\UE\T66\Saved\Codex\UI\HUDChromeMigration\UT66EnemyLockWidget\baseline_dump.json`

## Structure

- [ ] GameplayHUD.EnemyLock.Root | exists=true
- [ ] GameplayHUD.EnemyLock.Root | type=SBox
- [ ] GameplayHUD.EnemyLock.Bullseye | exists=true
- [ ] GameplayHUD.EnemyLock.Bullseye | type=ST66EnemyBullseyeWidget

## Geometry

- [ ] GameplayHUD.EnemyLock.Root | x=0.0000 | 0.005
- [ ] GameplayHUD.EnemyLock.Root | y=0.0000 | 0.005
- [ ] GameplayHUD.EnemyLock.Root | w=0.0275 | 0.005
- [ ] GameplayHUD.EnemyLock.Root | h=0.0489 | 0.005
- [ ] GameplayHUD.EnemyLock.Bullseye | x=0.0000 | 0.005
- [ ] GameplayHUD.EnemyLock.Bullseye | y=0.0000 | 0.005
- [ ] GameplayHUD.EnemyLock.Bullseye | w=0.0275 | 0.005
- [ ] GameplayHUD.EnemyLock.Bullseye | h=0.0489 | 0.005

## Colors

- [ ] GameplayHUD.EnemyLock.Root | state=Default
- [ ] GameplayHUD.EnemyLock.Bullseye | state=Default

## Content

- [ ] GameplayHUD.EnemyLock.Root | role=GameplayStateIndicatorRoot
- [ ] GameplayHUD.EnemyLock.Bullseye | role=GameplayStateIndicator
- [ ] GameplayHUD.EnemyLock.Root | is_label=false
- [ ] GameplayHUD.EnemyLock.Bullseye | is_label=false

## Interactivity

- [ ] GameplayHUD.EnemyLock.Root | has_click_handler=false
- [ ] GameplayHUD.EnemyLock.Root | hover_capable=false
- [ ] GameplayHUD.EnemyLock.Bullseye | has_click_handler=false
- [ ] GameplayHUD.EnemyLock.Bullseye | hover_capable=false
