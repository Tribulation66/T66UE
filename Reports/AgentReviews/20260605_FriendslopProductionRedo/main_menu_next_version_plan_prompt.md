Original user request:

Well hold on, I feel like you are answering these questions to conceptually and broadly and not really considering the context, every single ui element we have or at least 90% of them for the main menu screen were they not produced through imagegen? How else would we produce a button for example? I get what you mean about you take that image gen product and then create the actual slate umg buttons. I agrre on the next step, but lets not forget the other points because this was a clarification. This was your original assesment: Next step: stop iterating the crop/inpaint asset pipeline. Pass12 should be treated as diagnostic evidence, not a base for pass13.

The right next move is:

Pick the production method. My recommendation is native Slate/token-driven UI, since this screen is already live Slate and needs real states, sizing, text, icons, hover/click behavior, and dynamic data. Use Pillow/OpenCV/skimage only for measuring and verifying captures, not creating runtime button art.

Write a component contract before touching code. For each offender: topbar icon buttons, achievements tab, primary/secondary CTA, leaderboard row, checkbox, online header, invite button, friend rows. Define: owner of text/icon, exact size, font size, padding, colors, border/outline, fill, corner radius, hover/selected/disabled states.

Rebuild the worst elements natively first, not the whole screen. Start with:

topbar icon buttons: no baked icon under live icon
achievements tab: measured text-fit rule
CTA buttons: native glossy layered brush/material, no masked center
leaderboard row: red outline + dark interior
high score checkbox: uniform rounded square
online header green dot + green invite state
Replace the verifier gate. PASS=251 is not enough. Add a material/state gate with side-by-side crops for each component and explicit PASS/FAIL rows for color, shape, state, content ownership, and text fit.

Only then do a new capture. The next capture should prove a small native component slice first. If that slice still fails visually, we fix the method before touching the rest of the screen.

So the immediate next action, if we proceed, should be: freeze screenshot-inpaint runtime generation and create a native Slate component/token spec for those six failing UI families, then implement only that first component slice. So rework this into an actual implementation plan. Which you will do for the new version of the main menu screen

Task contract:

Working task:
Operator: Codex
Validator: Claude
Scope: Produce a concrete implementation plan for the next FriendslopStyle Main Menu version. The plan must explicitly correct the prior over-broad wording: imagegen remains valid for clean blank runtime chrome plates/component families, but screenshot crop/inpaint is frozen, and Slate/UMG owns live layout, text, icons, states, and data. No code or asset implementation in this answer.
Stop condition: Deliver a specific plan with phases, files/systems, imagegen asset family rules, Slate integration targets, verification gates, risks, and acceptance criteria.

Relevant live repo rules and observations:

- Root AGENTS.md requires Operator/Validator review for every T66 prompt and routes UI reference fidelity work through UI instructions.
- .t66/operator-state.json selects Codex as Operator and Claude as Validator.
- UI/UI_AGENTS.md says FriendslopStyle may use generated raster chrome only through UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, decomposed into reusable sliced UI assets, not pasted full-screen mockups.
- UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md says runtime chrome generation must create blank standalone plates one component family at a time, with no labels/names/scores/fake text, clean front-on orthographic view, alpha-clean transparent PNG, matched states, local contact sheets, slice specs, and Slate-owned live content.
- UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md says structured PASS counts do not prove visual fidelity; FriendslopStyle needs a blocking visual scorecard/contact sheet.
- UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md requires parent containment and stable min widths/heights for controls; clipped controls cannot be accepted.
- Current code surfaces that need plan-level targeting:
  - Source/T66/UI/Style/T66FriendslopStyle.h/cpp: ET66FriendslopChrome, GetChromeBrush, MakeButton, MakeFixedImage, chrome descriptors and draw modes.
  - Source/T66/UI/T66FrontendTopBarWidget.cpp: MakeIconActionButton, AchievementsRect, CategoryTabFontSize, category tabs, ticket canvas, power/settings/language buttons.
  - Source/T66/UI/Screens/T66MainMenuScreen.cpp: MakeCtaButton, MakeSearchField, MakeFriendGroupToggle, MakeFriendRow, MakeLeftPanel, MakeCtaStack.
  - Source/T66/UI/Components/T66FlatLeaderboardPanel.cpp/h: BuildFilterButton, BuildTimeButton, BuildMetricCheckButton, BuildLeaderboardRow.
- Known failures the plan must solve:
  - icon-on-icon on topbar buttons because generated plate/glyph ownership and live glyph ownership were not cleanly separated.
  - achievements label does not fit the button.
  - central CTA buttons look masked/smudged, as if text was painted out and new text written on top.
  - leaderboard local row uses red-filled interior but reference requires red outline with dark interior.
  - high score checkbox shape/color is not the uniform rounded square seen in reference.
  - friends panel lacks online green dot and green invite state in the visible result.
  - rows/content overflow inside panel cannot be accepted.

Please give an independent plan-focused answer. Do not edit files. Do not run mutating commands. Keep it practical and specific to this repo, and include any corrections to the plan if Codex should avoid a wrong framing.
