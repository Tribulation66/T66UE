# Phase 1 Stage Preflight

Date: 2026-05-28T11:04:32.5349183-03:00

## Staged Executable

- Path: $Exe
- SHA256: $(Microsoft.PowerShell.Commands.FileHashInfo.Hash)
- LengthBytes: $(C:\UE\T66_B11B12_Worktree\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe.Length)
- LastWriteTimeUtc: $(C:\UE\T66_B11B12_Worktree\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe.LastWriteTimeUtc.ToString('o'))

## Hero_2_Chad Content

- Source sentinel exists: True
- Sentinel count in source folder: 8
- Cook log: $CookLog
- Hero_2_Chad warnings found: 6

## Notes

No Hero_2_Chad missing-load warnings were found in the cook log. The cook log still contains unrelated companion AnimatedToonStyle dependency warnings; those are pre-existing content completeness noise outside this pass and did not reference Hero_2_Chad.

## First AnimatedToonStyle Matches

``text
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/AM_Companion_01_Idle which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/AM_Companion_01_Jump which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/AM_Companion_01_Roll which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/AM_Companion_01_Walk which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/SK_Companion_01 which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/Default/AnimatedToonStyle/Textures/T_Companion_01_Animated_BaseColor which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/AM_Companion_01_DemoSkin_Idle which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/AM_Companion_01_DemoSkin_Jump which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/AM_Companion_01_DemoSkin_Roll which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/AM_Companion_01_DemoSkin_Walk which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/SK_Companion_01_DemoSkin which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_01/DemoSkin/AnimatedToonStyle/Textures/T_Companion_01_DemoSkin_Animated_BaseColor which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/AM_Companion_02_Idle which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/AM_Companion_02_Jump which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/AM_Companion_02_Roll which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/AM_Companion_02_Walk which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/SK_Companion_02 which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/Default/AnimatedToonStyle/Textures/T_Companion_02_Animated_BaseColor which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/DemoSkin/AnimatedToonStyle/AM_Companion_02_DemoSkin_Idle which does not exist.
LogCook: Display: Package /Game/Data/DT_CharacterVisuals has a dependency on package /Game/Characters/Companions/Companion_02/DemoSkin/AnimatedToonStyle/AM_Companion_02_DemoSkin_Jump which does not exist.
``
