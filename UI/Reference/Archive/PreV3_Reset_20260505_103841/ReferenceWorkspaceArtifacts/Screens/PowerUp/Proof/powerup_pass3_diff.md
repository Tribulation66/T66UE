# PowerUp pass 3 packaged diff

Reference:
- `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUp.png`

Packaged proofs:
- Baseline: `C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_baseline_1920x1080.png`
- Pass 1: `C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_pass1_1672x941.png`
- Pass 2: `C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_pass2_1672x941.png`
- Final pass: `C:\UE\T66\UI\Reference\Screens\PowerUp\Proof\powerup_pass3_1672x941.png`

Accepted PowerUp-owned runtime chrome:
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_hover.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_pressed.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_disabled.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Buttons\powerup_buttons_pill_selected.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_upgrade_card_normal.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_item_art_well.png`
- `C:\UE\T66\SourceAssets\UI\Reference\Screens\PowerUp\Panels\powerup_panels_info_strip.png`
- Existing PowerUp assets retained: fullscreen shell, row shell, controls sheet, coupon icon, scene plate.

Generated candidates:
- No new imagegen candidates were emitted in this pass. The available non-inline imagegen route had no `OPENAI_API_KEY`, and inline generated outputs were disallowed by the task. Existing generated/shared chrome was duplicated into the PowerUp runtime folder and then routed through PowerUp-owned paths.

What changed from baseline:
- Header now uses a centered PowerUp title with the tab row below it.
- Tabs and all PowerUp buttons route to local PowerUp-owned sliced pill assets, with live text over the plate.
- Cards route to local PowerUp-owned parchment/card chrome and render as 9-slice panels.
- Item wells route to local PowerUp-owned frame chrome and keep item art live.
- Info strip routes to a local PowerUp-owned 9-slice strip.
- Horizontal scrollbars use the PowerUp controls sheet instead of default Slate chrome.
- Card dimensions were reduced to match the reference card count and spacing at 1672x941.

Remaining differences:
- Top navigation is still current runtime live chrome/data, not reference-placeholder data: coupon count and portrait differ from the reference.
- The reference uses placeholder item labels and empty item wells. Runtime keeps live PowerUp labels, costs, coupon icons, and diploma art per the live-data rule.
- The fullscreen shell and card ornaments are close but not exact: the current PowerUp-owned/promoted assets have heavier gold/corner detail than the reference.
- The title area is vertically close but constrained by the current runtime top bar height, which is taller than the reference top bar.

Verification:
- `Build.bat T66 Win64 Development C:\UE\T66\T66.uproject -WaitMutex -FromMsBuild -SingleFile=C:\UE\T66\Source\T66\UI\Screens\T66PowerUpScreen.cpp` succeeded.
- Full stage/cook/package succeeded with `_CL_=/wd4458` to avoid an unrelated `T66AccountStatusScreen.cpp` shadow warning blocking packaging.
- Final capture succeeded from `C:\UE\T66\Saved\StagedBuilds\Windows\T66\Binaries\Win64\T66.exe`.
