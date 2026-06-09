You are right to call out the wording. For FriendslopStyle, "rebuild the button" does not mean "stop using imagegen for the button art." The actual production split is:

- imagegen or Photoshop-equivalent source-art work produces the blank, clean, reference-matched chrome plate family for the button/panel/row/etc.
- Slate/UMG turns that plate into the real runtime component: live text, live icons, live data, padding, fit rules, states, click behavior, tags, containment, and verification.

So the corrected plan for the next Main Menu version is this:

1. Freeze the bad asset method.
   - Do not use pass12 crop/inpaint output as a base for pass13.
   - Keep pass12 only as diagnostic evidence of what failed: cut-off middles, painted-over labels, baked glyphs, wrong shapes/colors, and containment misses.
   - Pillow/OpenCV/skimage are allowed for alpha cleanup inspection, crop sheets, fit measurements, and verifier/contact sheets. They are not the production tool for removing text from screenshots or inventing final button interiors.

2. Write the pass13 component contract before touching code or assets.
   - Artifact: `UI/FriendslopStyle/Elements/main_menu_pass13_component_contract.md`.
   - For each component family, define:
     - reference crop path and intended runtime tag
     - imagegen/source-art ownership: blank plate, static decorative marks only, no live labels/data/glyphs
     - Slate ownership: text, icons, live player/friend/score data, state, layout, hit rect
     - exact normal size, min size, max/wide behavior, content padding, font size, icon size, z-order
     - state family required: normal, hover, pressed, selected, disabled, online/offline where applicable
     - expected visual properties: fill, outline, gloss, bevel thickness, corner/cap shape, shadow, material density
     - verifier crop name and PASS/FAIL criteria
   - Families in scope for first implementation slice:
     - topbar icon buttons
     - achievements tab
     - CTA primary and secondary buttons
     - leaderboard local row
     - high-score checkbox
     - friends online header, invite/offline buttons, and friend rows

3. Regenerate or author clean chrome families, not screenshot-derived fixes.
   - Source path: `SourceAssets/UI/FriendslopStyle/MainMenu/Pass13/`.
   - Prompt path: `UI/FriendslopStyle/SourcePrompts/MainMenu/pass13_*.md`.
   - Runtime path after acceptance: `RuntimeDependencies/T66/UI/FriendslopStyle/MainMenu/`.
   - Rule per generated family:
     - front-on orthographic chrome only
     - transparent alpha or removable matte only as intermediate
     - no text, fake text, labels, names, scores, icons, skulls, stars, search glyphs, or baked player data unless the contract explicitly says a decorative glyph is part of the plate
     - generate matched state variants together, not one-off mismatched pieces
     - create min/normal/wide contact sheet before Unreal import
     - reject any plate with visible text-smear, center masking, split/cut middle, baked glyph contamination, or mismatched material density
   - For this reference, the CTA buttons, topbar tabs, rounded icon plates, leaderboard row shell, checkbox, invite/offline buttons, and friend row shells should be treated as authored/generated chrome plates unless a native-only construction can pass the side-by-side crop gate. Slate-native composition is the runtime method, not a replacement for the premium plate art.

4. Update the Friendslop style registry and slice specs.
   - Code surfaces:
     - `Source/T66/UI/Style/T66FriendslopStyle.h`
     - `Source/T66/UI/Style/T66FriendslopStyle.cpp`
   - Add or replace descriptors for pass13 role-specific assets instead of reusing contaminated generic ones.
   - Use `DrawAs=Box` and real nonzero margins only for assets that pass slice tests.
   - Use size-specific/fixed plates where slicing destroys highlights or caps.
   - Create/refresh `UI/FriendslopStyle/SliceSpecs/main_menu_pass13_slice_specs.md`.
   - Specific cleanup:
     - topbar icon backgrounds must be blank plates if live glyphs are rendered on top
     - CTA plates must be clean blank button bodies if live skull icons/text are rendered on top
     - leaderboard local row must become red outline plus dark interior, not red-filled interior
     - checkbox checked/empty must be uniform rounded square states

5. Implement the runtime component slice in targeted code.
   - `Source/T66/UI/T66FrontendTopBarWidget.cpp`
     - `MakeIconActionButton`: one owner for glyphs. Use blank generated plate plus live glyph, or baked glyph plate with no live glyph. For this pass choose blank plate plus live glyph.
     - `AchievementsRect` and `CategoryTabFontSize`: replace fixed text assumptions with a measured fit rule for the `ACHIEVEMENTS` label, including min-width/compact fallback.
   - `Source/T66/UI/Screens/T66MainMenuScreen.cpp`
     - `MakeCtaButton`: use clean CTA plate, live text, live skull slots if needed, no plate with skull/text remnants under live content.
     - `MakeSearchField`: use a deliberate search glyph or no glyph, not a question-mark placeholder if the reference expects search.
     - `MakeFriendGroupToggle`: add online green status dot and keep header text/count live.
     - `MakeFriendRow`: lock row width/height inside left panel content bounds; use live avatar, name, level, star, and action button; green invite plate for online, dark offline plate for offline.
   - `Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp`
     - `BuildLeaderboardRow`: make local row red outline/dark interior with live rank/name/score.
     - `BuildMetricCheckButton`: use uniform checked/empty rounded-square assets and stable box size.
     - `BuildFilterButton`/dropdowns: enforce icon ownership and text fit the same way as topbar.

6. Replace the verifier gate for this pass.
   - Artifact: `Saved/Codex/UI/FriendslopStyle/MainMenu/pass13_material_state_scorecard.md`.
   - A numeric `VerifyUIFidelity.py` PASS count is not sufficient.
   - Required scorecard rows:
     - topbar icons: no icon-on-icon, correct plate shape, correct glyph owner
     - achievements tab: text fits and no clipped/overflowing label
     - CTA buttons: no masked center, no smudged remnants, correct red/dark rubber material
     - leaderboard row: red outline and dark interior
     - checkbox: uniform rounded square checked/empty states
     - friends header: online green dot present
     - invite/offline buttons: green invite and dark offline states visible, text fits
     - friend rows/leaderboard rows: contained inside panel body
   - Each row must include reference crop, current crop, verdict, and reason.

7. Capture only after the component slice is integrated.
   - Run focused compile first.
   - Capture through the Unreal-owned UI capture path, not desktop screenshots.
   - Produce:
     - fresh capture PNG
     - fresh dump JSON
     - verifier report
     - side-by-side crop/contact sheet
     - visual scorecard
   - The first pass13 capture is allowed to prove only this component slice. If the slice still has the same method failure, stop and fix the method before expanding to the rest of the screen.

8. Acceptance criteria for the next version slice.
   - No screenshot-inpaint production assets.
   - No baked live text/icons underneath live text/icons.
   - No manual bars, painted-over centers, or smudged masks.
   - No cut-in-half chrome or missing middle bands.
   - Text fits the component at the declared minimum size.
   - Rows and child controls fit inside their parent panels with containment checks.
   - The specific color/shape/detail discrepancies called out by the user are corrected, not averaged away.
   - Visual scorecard is `Result: PASS` for the component slice before scaling the approach.

9. What I will not do in the next implementation pass.
   - I will not generate another full-screen reference and paste/crop pieces out of it.
   - I will not inpaint text out of screenshots and call the result a clean component.
   - I will not cover bad baked labels with a manual bar or overlay.
   - I will not accept `PASS=251` or compile success as proof of visual fidelity.
   - I will not implement the whole screen before the first component slice proves the production method.
