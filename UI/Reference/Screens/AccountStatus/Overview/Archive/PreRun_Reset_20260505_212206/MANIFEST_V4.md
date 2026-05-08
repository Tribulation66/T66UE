# AccountStatusOverview MANIFEST_V4

## Pass 00 Preflight And Capture

- Target: AccountStatusOverview
- Base screen/modal: AccountStatus
- Target state: Account overview tab
- Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\AccountStatus.png`
- Workspace folder: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview`
- Runtime asset folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Overview`
- Common runtime folder: `C:\UE\T66\SourceAssets\UI\Reference\Screens\AccountStatus\Common`
- Source file checked: `C:\UE\T66\Source\T66\UI\Screens\T66AccountStatusScreen.cpp`
- Required docs checked:
  - `C:\UE\T66\UI\Reference\SCREEN_MODAL_TASK.md`
  - `C:\UE\T66\Docs\UI\UI_GENERATION.md`
- Capture script checked: `C:\UE\T66\Scripts\CaptureT66UIScreen.ps1`
- Local executable checked: `C:\UE\T66\Binaries\Win64\T66.exe`

### Reference Geometry Map 1920x1080

Coordinates are approximate visual bounds from the reference image.

| Element | X | Y | W | H | Resize role | Asset family |
|---|---:|---:|---:|---:|---|---|
| Shared top bar | 0 | 0 | 1920 | 126 | frozen/out of scope | shared top bar/header |
| Account title | 810 | 143 | 300 | 64 | live text | title text |
| Overview tab | 645 | 216 | 300 | 52 | horizontal sliced plate | selected tab/button |
| History tab | 974 | 216 | 294 | 52 | horizontal sliced plate | unselected tab/button |
| Main parchment content shell | 52 | 276 | 1752 | 770 | 9-slice | full-screen paper panel |
| Left profile card | 82 | 305 | 710 | 194 | 9-slice | parchment card |
| Avatar slot | 108 | 331 | 152 | 150 | fixed/9-slice | square slot frame |
| XP progress track | 288 | 423 | 376 | 20 | horizontal sliced/tiled | progress track/fill |
| Account status card | 82 | 508 | 710 | 194 | 9-slice | parchment card |
| Account progress card | 82 | 694 | 710 | 295 | 9-slice | parchment card |
| Progress rows | 108 | 753 | 638 | 172 | repeated live rows | text plus progress bars |
| Right table panel | 838 | 305 | 962 | 684 | 9-slice | parchment card |
| Dropdown left | 866 | 305 | 423 | 55 | horizontal 3-slice/9-slice | dark brown dropdown |
| Dropdown right | 1347 | 305 | 408 | 55 | horizontal 3-slice/9-slice | dark brown dropdown |
| Highest score table | 846 | 374 | 910 | 298 | 9-slice with live rows | table shell/dividers |
| Best speed run table | 846 | 685 | 910 | 293 | 9-slice with live rows | table shell/dividers |
| Vertical scrollbar | 1822 | 279 | 35 | 743 | vertical 3-slice | scrollbar track/thumb/caps |

### Capture Attempts

1. Failed. Command:
   `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Proof\AccountStatusOverview_pass00_working_1920x1080.png -ExtraArgs "-T66AccountTab=Overview"`
   Result: timed out after the script timeout; output file was not created.
2. Failed. Command:
   `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Proof\AccountStatusOverview_pass00_retry2_working_1920x1080.png -ExtraArgs "-T66AccountTab=Overview -T66AutoScreenshotDelay=6"`
   Result: timed out; launch line contained both `-T66AutoScreenshotDelay=3.5` and `-T66AutoScreenshotDelay=6`; output file was not created.
3. Failed. Command:
   `powershell -ExecutionPolicy Bypass -File C:\UE\T66\Scripts\CaptureT66UIScreen.ps1 -Exe C:\UE\T66\Binaries\Win64\T66.exe -Screen AccountStatus -ResX 1920 -ResY 1080 -DelaySeconds 10 -TimeoutSeconds 90 -Output C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Proof\AccountStatusOverview_pass00_retry3_working_1920x1080.png -ExtraArgs "-T66AccountTab=Overview"`
   Result: timed out after 90 seconds; output file was not created.

All three expected output files were checked with `Test-Path` and returned `False`.

## Pass 01 Imagegen

- Built-in imagegen used: yes
- Built-in source output: `C:\Users\DoPra\.codex\generated_images\019df98b-1b99-7080-8c7c-aa592700aa72\ig_0348cb3bf7029c730169fa417a1cb88198a7f1b3e4ac490b75.png`
- Candidate copy: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Working\Pass_01\Candidates\accountstatus_overview_textfree_sheet_pass01.png`
- Rejected archive copy: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\Rejected\Pass_01\accountstatus_overview_textfree_sheet_pass01_REJECTED.png`
- Rejection note: `C:\UE\T66\UI\Reference\Screens\AccountStatus\Overview\Archive\Rejected\Pass_01\REJECTION_NOTE.txt`

### Sprite Sheet Quality Gate

Status: FAIL

Reason: the sheet is text-free, but it is materially different from the reference. It is darker, more ornate, heavier-bordered, higher-contrast, more beveled, and uses denser corner/trim decoration than the quiet reference UI chrome. It must not be sliced, promoted, or wired.

### Source And Runtime Changes

- Accepted runtime assets: none
- Source files changed: none
- Build command/status: not run; no source or runtime assets were changed.

### Remaining Differences

Not visually comparable from runtime because capture is blocked: three working-capture attempts failed to create a screenshot.

### Next Action

Resolve the working capture blocker first, then regenerate a stricter restrained text-free sheet and continue the V4 visual loop.
