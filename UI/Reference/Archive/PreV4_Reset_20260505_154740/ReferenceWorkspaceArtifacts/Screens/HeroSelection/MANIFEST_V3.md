# HeroSelection V3 Manifest

## Geometry Map

Reference image: `C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\HeroSelection.png`

Reference native size: 1672 x 941. Working proof target: 1920 x 1080. Scale used for 1920 proof comparison: 1.148.

| Family | Reference native x,y,w,h | 1920 proof x,y,w,h | Resize role |
| --- | --- | --- | --- |
| Left owned side panel | 15,24,512,754 | 17,28,588,866 | 9-slice black/gold panel |
| Center hero preview viewport | 540,25,675,753 | 620,29,775,865 | live 3D preview, no baked art |
| Right owned side panel | 1228,24,428,754 | 1410,28,491,866 | 9-slice black/gold panel |
| Hero carousel | 543,24,670,84 | 623,28,769,96 | live portrait row with fixed slots |
| Left bottom party bar | 15,785,628,133 | 17,901,721,153 | 9-slice black/gold bar |
| Center bottom companion bar | 649,786,430,132 | 745,902,494,152 | 9-slice black/gold bar |
| Right bottom run bar | 1085,785,572,133 | 1245,901,657,153 | 9-slice black/gold bar |
| Parchment preview panel | 1245,108,395,271 | 1429,124,453,311 | 9-slice parchment |
| Parchment medal row | 1245,385,395,59 | 1429,442,453,68 | horizontal/9-slice parchment row |
| Parchment stats panel | 1245,450,395,189 | 1429,517,453,217 | 9-slice parchment |
| Parchment ability row | 1245,649,395,89 | 1429,745,453,102 | horizontal/9-slice parchment row |
| Bottom buttons | variable 74-263 wide, 49-78 high | variable 85-302 wide, 56-90 high | horizontal sliced buttons |
| Square slots | about 72 x 72 | about 83 x 83 | fixed image slots |

## Pass 00

- Generated candidate paths: none.
- Accepted runtime paths: none.
- Source files changed: none in this pass.
- Build command/status: not run.
- Screenshot proof path: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass00_working_1920x1080.png`
- Remaining differences:
  - asset: target runtime folder was empty after reset, so the screen relied on missing/stale chrome and produced over-ornate or fallback visual areas.
  - layout: left panel starts too far right, right panel is too wide, bottom bars are too shallow and split differently, center preview is too cropped.
  - live-data: player names, stats, owned/equipped state, and preview body differ from the locked reference and remain live runtime data.
  - approved shared-top-bar: none identified for this screen; the carousel belongs to this screen.
- Exact next action: generate reference-derived text-free sheet, promote target-owned components, adjust layout, build, and capture pass 01.

## Pass 01

- Generated candidate paths:
  - `C:\Users\DoPra\.codex\generated_images\019df885-d184-7611-8bde-8da75c4a308b\ig_055c5283c8ff78d40169f9fdda853c819a975783774201ed5d.png` rejected: checkerboard was baked as opaque pixels, not usable for runtime transparency.
  - `C:\Users\DoPra\.codex\generated_images\019df885-30f1-7a73-a068-6206269199c4\ig_063353b3563f86160169f9fe61c708819a9a08ef27e1f0da0a.png`
  - `C:\UE\T66\UI\Reference\Screens\HeroSelection\Working\Pass_01\Candidates\heroselection_imagegen_pass01_textfree_chromakey_sheet.png`
- Accepted runtime paths:
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_normal.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_hover.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_selected.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_pressed.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_pill_disabled.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_normal.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_hover.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_selected.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_pressed.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Buttons\heroselection_buttons_imagegen20260505_slim_square_disabled.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_tall_panel.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_wide_panel.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_panel.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Panels\heroselection_panels_imagegen20260505_slim_parchment_row.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_rail.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_scrollbar_thumb.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Controls\heroselection_controls_imagegen20260505_slim_dropdown_field.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_normal.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_portrait_slot_selected.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Slots\heroselection_slots_reference_square_slot_frame_normal.png`
  - `C:\UE\T66\SourceAssets\UI\Reference\Screens\HeroSelection\Icons\heroselection_iconsgenerated_icon_07_coupon_ticket_white_v1.png`
- Source files changed:
  - `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Private.h`
  - `C:\UE\T66\Source\T66\UI\Screens\HeroSelection\T66HeroSelectionScreen_Build.cpp`
- Compile-only unrelated fix:
  - `C:\UE\T66\Source\T66\Gameplay\T66GameMode.h`: added `Gameplay/T66StartGate.h` include after UHT failed on `TObjectPtr<AT66StartGate>` with `Expected name`.
- Build command/status:
  - Attempt 1: failed on unrelated `T66GameMode.h(300): Expected name`.
  - Compile-only fix applied: included `Gameplay/T66StartGate.h`.
  - Attempt 2: failed on broad unrelated gameplay/API drift outside this target, including removed casino/trickster/shop/wheel/fountain declarations still referenced from `T66GameMode_*`, `T66PlayerController_Overlays.cpp`, `T66GamblerNPC.cpp`, `T66WheelSpinInteractable.cpp`, `T66LabOverlayWidget.cpp`, and `T66CollectorOverlayWidget.cpp`.
  - Build blocker classification: external to this target and too broad for a compile-only UI-pass fix.
- Screenshot proof path: `C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass00_working_1920x1080.png` remains the latest successful working proof. Pass 01 capture attempts did not produce a fresh screenshot after the failed build.
- Remaining differences: pass 00 visual differences remain the last proven differences until a successful pass 01 executable/capture is available.
- Approved live-data/top-bar-shared differences: live player name, stats, owned/equipped/balance values, party identity, and 3D preview body can differ if layout/chrome matches.
- Exact next action: resolve the unrelated gameplay build blockers, rebuild normally, capture `C:\UE\T66\UI\Reference\Screens\HeroSelection\Proof\HeroSelection_pass01_working_1920x1080.png`, then continue visual iteration.
