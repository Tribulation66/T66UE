# Pre-Change Screen Capture Packet

## Task Contract

Working task: capture current Unreal-owned screenshots for Hero Selection, Companion Selection, Gambler, Vendor, Idol Altar Selection, and Weapon Altar Selection screens before later changes.
Operator: Codex.
Validator: Claude.
Scope: screenshot/proof artifacts only under `Reports/Proof/UI/PreChangeScreens_20260530_213203`; no code, content, config, or save edits requested.
Stop condition: all requested screenshots exist, dimensions are verified, and Claude validates the packet or a blocker is reported.

## Instructions Followed

- Root instructions: `C:\UE\T66\AGENTS.md`.
- Report routing: `C:\UE\T66\Reports\AGENTS.md`.
- UI routing: `C:\UE\T66\UI\UI_AGENTS.md`.
- Capture procedure reference: `C:\UE\T66\UI\Instructions\UI_FIDELITY_LOOP_INSTRUCTIONS.md`.
- Operator state: `.t66\operator-state.json` reported `operator=Codex`, `validator=Claude`.
- Mini/minigame scope was not inspected or included.

## Capture Commands

Frontend captures:

```powershell
& 'C:\UE\T66\Scripts\CaptureT66UIScreen.ps1' -Screen HeroSelection -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\hero_selection_screen.png' -DelaySeconds 6 -TimeoutSeconds 180
& 'C:\UE\T66\Scripts\CaptureT66UIScreen.ps1' -Screen CompanionSelection -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\companion_selection_screen.png' -DelaySeconds 6 -TimeoutSeconds 180
```

Gameplay overlay captures:

```powershell
& 'C:\UE\T66\Scripts\CaptureT66UIWidget.ps1' -Target 'Class=UT66IdolAltarOverlayWidget' -CaptureMode idol -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\idol_altar_selection_screen.png' -Dump 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\idol_altar_selection_screen_dump.json' -DelaySeconds 6 -TimeoutSeconds 180
& 'C:\UE\T66\Scripts\CaptureT66UIWidget.ps1' -Target 'Class=UT66WeaponAltarOverlayWidget' -CaptureMode weapon -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\weapon_altar_selection_screen.png' -Dump 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\weapon_altar_selection_screen_dump.json' -DelaySeconds 6 -TimeoutSeconds 180
& 'C:\UE\T66\Scripts\CaptureT66UIWidget.ps1' -Target 'Class=UT66CasinoOverlayWidget' -CaptureMode vendor -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\vendor_screen.png' -Dump 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\vendor_screen_dump.json' -DelaySeconds 6 -TimeoutSeconds 180
& 'C:\UE\T66\Scripts\CaptureT66UIWidget.ps1' -Target 'Class=UT66CasinoOverlayWidget' -CaptureMode casinogambling -Output 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\gambler_screen.png' -Dump 'C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\gambler_screen_dump.json' -DelaySeconds 6 -TimeoutSeconds 180
```

## Screenshot Artifacts

| Requested screen | Screenshot | Dimensions | Bytes |
|---|---|---:|---:|
| Hero Selection Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\hero_selection_screen.png` | 1920x1080 | 1193800 |
| Companion Selection Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\companion_selection_screen.png` | 1920x1080 | 999787 |
| Gambler Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\gambler_screen.png` | 1920x1080 | 275093 |
| Vendor Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\vendor_screen.png` | 1920x1080 | 237050 |
| Idol Altar Selection Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\idol_altar_selection_screen.png` | 1920x1080 | 649732 |
| Weapon Altar Selection Screen | `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\weapon_altar_selection_screen.png` | 1920x1080 | 440184 |

## Additional Artifacts

- `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\idol_altar_selection_screen_dump.json`
- `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\weapon_altar_selection_screen_dump.json`
- `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\vendor_screen_dump.json`
- `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\gambler_screen_dump.json`
- `C:\UE\T66\Reports\Proof\UI\PreChangeScreens_20260530_213203\.report-run.json`

## Verification Performed

- Confirmed staged executable exists at `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
- Each capture command completed successfully and printed `Captured <path>`.
- Overlay captures also printed `Dumped <path>` for their widget dumps.
- Dimension check used `System.Drawing.Image` and confirmed every PNG is `1920x1080`.
- Visual sanity pass inspected all six images and confirmed each screenshot corresponds to the requested screen.

## Caveats

- This packet captures current screens only. It does not evaluate visual quality, performance, or correctness.
- Frontend screenshots were captured as screenshots only; overlay screenshots also have widget dumps because `CaptureT66UIWidget.ps1` requires a target and dump path for class-targeted capture verification.
