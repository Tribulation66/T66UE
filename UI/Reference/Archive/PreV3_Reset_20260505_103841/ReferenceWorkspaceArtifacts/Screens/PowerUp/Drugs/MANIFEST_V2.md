# PowerUpDrugs MANIFEST_V2

## Pass 00 - Reference Creation

Generated exact-state reference:
- C:\UE\T66\UI\generation\ReferenceFullScreens_2026-05-02_inventory_locked\PowerUpDrugs.png

State/layout source capture:
- C:\UE\T66\UI\Reference\Screens\PowerUp\Drugs\Proof\PowerUpDrugs_current_packaged_1920x1080.png

Built-in imagegen source:
- C:\Users\DoPra\.codex\generated_images\019df839-1ac5-7fa1-8e93-90bdec69dd35\ig_09e3ad4292cc4fc20169f9eb29b210819bb4f5d35ce882a4a0.png

Prompt summary:
- Generated a 1920x1080 PowerUp DRUGS / single-use reference using PowerUp.png as style anchor and current packaged DRUGS capture as layout source.

## Pass 01 - Initial Geometry And Differences

Reference geometry map at 1920x1080:
- Top shared chrome: x=0 y=0 w=1920 h=132, shared/top-bar freeze, out of scope for target-owned changes.
- PowerUp shell/background: x=18 y=132 w=1884 h=932, 9-slice/fullscreen background role.
- Title: center x=960 y=170 w~420 h~80, live text.
- Tabs: DIPLOMAS x~648 y~212 w~306 h~52, DRUGS x~970 y~212 w~306 h~52, horizontal 3-slice buttons, DRUGS selected.
- Hint strip: x~125 y~281 w~1670 h~60, horizontal/9-slice parchment strip, live placeholder text.
- Content rows: first row y~356 h~340, second row y~718 h~340 visible, vertical scroll content.
- Category panels: x~75 y~356 w~330 h~340, x~75 y~718 w~330 h~340, 9-slice dark framed panels, live placeholder category labels.
- Item cards: x~445 y~356 w~330 h~340 then +360 spacing, 9-slice parchment card, fixed art well, horizontal 3-slice BUY button.
- Scrollbar: x~1836 y~341 w~25 h~676, vertical 3-slice rail/thumb.

Current packaged geometry map at 1920x1080:
- Top shared chrome: x=0 y=14 w=1920 h=130, shared/top-bar freeze, out of scope.
- PowerUp shell: x=30 y=147 w=1860 h=913, target content starts too low and uses brighter/heavier frame.
- Title: x~785 y=172 w~350 h~70, close but lower/cleaner in reference.
- Tabs: x~637 y=271 w~648 h~54, lower than reference by ~59 px, DRUGS selected.
- Hint strip: x~103 y=350 w~1674 h~47, lower by ~69 px, too thin.
- Rows: first row y~437 h~350, second row y~855 h~350 clipped, lower by ~81 px.
- Category panels: x~87 y~437 w~332 h~335, ornate/heavier than reference.
- Item cards: x~445 y~437 w~300 h~335, too narrow vs reference cards and runtime text/names are live.
- Scrollbar: x~1833 y~347 w~18 h~697, close x but art heavier/thinner than reference.

Difference list before edits:
- layout: target content is shifted downward about 55-80 px versus the new reference.
- layout: item cards are narrower and row spacing differs; reference uses wider cards with balanced spacing.
- asset: runtime category panels/card frames are heavier and more ornate than reference-derived restrained chrome.
- asset: runtime top/background shell differs from new reference content shell; shared top-bar differences are frozen/out of scope.
- resize-contract: buttons/panels need fresh v2 explicit sliced/box contracts after asset reset.
- live-data: current package shows exact drug names/icons/costs; reference uses placeholder text/art. Live text/data differences are approved only after layout/assets match.
