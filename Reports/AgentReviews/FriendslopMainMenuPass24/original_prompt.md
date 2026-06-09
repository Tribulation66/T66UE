# Original User Request
Okay, so it's better, except there's an issue where the panel is in front of the background image. We need to bring the panel, no, sorry, it's the opposite. The background image of the statue is in front of the leaderboard panel. The leaderboard panel needs to be brought to the front, okay? And then the other issue is actually the global social streamers. I want them to be icons, not text. So let's fix that as well. And the, and the last thing is, I want the coupon icon at the top. Right now, it's this weird yellow thing. It doesn't even really look like a coupon. I wanted you to make it more coupon-like, like just a classic fair coupon. Okay, so let's make those changes, and then in the next answer, make, so make those changes, send me the new reference image, you know, do deploy a CLI for it, but send me that. And then I want you to, I mean, you already gave me the solution, so okay, that's fine. So then in this next pass, you're gonna make these changes to the reference image, okay, that I mentioned. And then in the same pass, you're gonna do another iteration, okay, implementing the solutions that you came up with. Okay, so it's like we did previously, it's in one answer. You're gonna do the updated reference image and then a full pass at the, at the iteration process to make the screen look like it.

# Task Contract
Working task: Update the FriendslopStyle Main Menu reference image with three requested changes, then run a complete implementation iteration against the updated reference.
Operator: Codex
Validator: Claude
Scope: Reference regeneration via separate local Codex CLI account-backed built-in imagegen worker; archive/promote updated reference; then full five-family FriendslopStyle Main Menu iteration: assess all five families, generate runtime assets via one CLI worker per failed family, implement generated assets, run sizing/fitting correction, run wiring/functionality gate, capture/dump/contact evidence, and report process coverage. No native goal tools. No git/release operations. No main-chat image generation. No OPENAI_API_KEY/API/web/manual pixel repair fallback.
Stop condition: New current reference is promoted, generated runtime assets for failed families are implemented, sizing/fitting and wiring/functionality are attempted, fresh capture/dump/contact evidence and worker records are produced, or a hard blocker is documented.

# Current Reference Before This Pass
C:\UE\T66\UI\FriendslopStyle\Reference\MainMenu\Current\main_menu_reference_03_equal_width_right_panel_toggle_above_cli.png

# Required reference changes
1. Fix right-side layering: statue/background must not appear in front of the leaderboard panel; leaderboard and toggle panel must be clearly foreground UI.
2. Replace GLOBAL/SOCIAL/STREAMERS text in the small top leaderboard filter panel with icons, not text.
3. Make the topbar coupon icon look like a classic fair/carnival coupon/ticket, not a weird yellow abstract mark.

# Process constraints
- Follow AGENTS.md, OPERATOR_VALIDATOR_PROTOCOL.md, UI/UI_AGENTS.md, UI/Instructions/UI_FIDELITY_LOOP_INSTRUCTIONS.md, UI/Instructions/UI_LAYOUT_AND_SIZING_INSTRUCTIONS.md, UI/FriendslopStyle/README.md, UI/FriendslopStyle/FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md, and UI/FriendslopStyle/Screens/MainMenu docs.
- FriendslopStyle visual final acceptance is user-owned. Codex should report process coverage and wiring/functionality PASS/FAIL only, not FULL/PARTIAL or visual scorecard Result.
- Image generation must be done through separate local Codex CLI workers using account-backed built-in imagegen. Each worker records request, start time/run id, input paths, logs/transcript, final status, output PNG path, token count and SHA-256 when available.
- Runtime UI cannot paste the full reference image, cannot bake labels/player data/scores/localized text/action labels into plates, and cannot use reference crops as runtime assets.
- The full implementation iteration must evaluate the five families: TopBar, LeftSocialPanel, RightLeaderboardPanel, CenterButtonStack, Background.

# Validator request
Produce an independent read-only answer: identify important constraints, likely file/code areas to inspect, likely pitfalls, and what evidence the final answer should include. Do not mutate files.

Result: OK or Result: NEEDS_USER
