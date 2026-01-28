# TRIBULATION 66 - GAME BIBLE

Visual-first UI + gameplay descriptions (for design lock-in) + internal inventories (for planning).

# 1. FRONTEND - BEFORE THE RUN

## 1.1 BOOT INTRO

(Appears automatically when launching the game. No buttons, no version text, no skip prompt.)

The Boot Intro is not a video cutscene. It is a fast, full-screen “faux intro” where the Main Menu visually assembles itself while the game finishes loading.

The screen begins as a clean, dark background with a soft vignette. Over roughly one to two seconds (or however long loading takes), the menu elements slide and fade into place: a tall button column forms on the left, a large leaderboard panel forms on the right, and the small system buttons settle into the corners. The movement feels snappy and confident, like the UI is being built in real-time.

There is no “Press Any Key”, no “Skip”, and no build/version number. As soon as the assembly animation finishes, the player is immediately left on the fully usable Main Menu.

## 1.2 MAIN MENU

(Primary hub after Boot Intro. Left = navigation buttons. Right = the full leaderboard (not a preview).)

The Main Menu is a full-screen layout split into two clear regions. The left region is a clean vertical navigation stack, and the right region is the game’s full leaderboard interface where players can browse Top 10 runs immediately.

Left Region (Navigation): On the left side, four large rectangular buttons are stacked vertically with consistent spacing and equal sizing. From top to bottom the labels read: New Game, Load Game, Settings, Achievements. The buttons are bold, readable, and designed to be the most obvious interactive elements on the screen. DEV BUILD NOTE: During development, an additional The Lab button may appear in this left stack as a temporary shortcut into the Practice Lab for rapid testing. This dev-only entry point is removed/hidden for release builds.

Corner Utilities: In the bottom-left corner, separate from the main button stack, there is a small icon-only Language button (no text). In the top-right corner there is a small Quit button, placed like a system control (similar to Dota-style placement) so it feels separate from the main navigation.

Right Region (Leaderboard - this is the leaderboard itself): The entire right half is a dedicated leaderboard panel. At the very top of this panel is a small button labeled Account Status. This button is not always visible - it only appears if the account has a run currently flagged under suspicion of cheating (and therefore submissions may be suspended or under review).

Leaderboard Filters: Under the Account Status area is a tight set of toggle rows. The first row is a three-way toggle: Global, Friends, Streamers (Twitch icon). Each toggle is highlighted when selected, and arranged as clean side-by-side choices.

Leaderboard Dropdowns: Below the toggle rows are two dropdown menus side-by-side. The left dropdown selects the party size board (Solo Runs, Duo Runs, Trio Runs). The right dropdown selects difficulty (Easy, Medium, Hard, Very Hard, Impossible, Perdition, Final).

Leaderboard Rows: Below the dropdowns is a list of exactly 10 ranked rows. If the local player is not present in the Top 10, an additional 11th row shows the local player’s current Global ranking and score for comparison (this 11th-row rule applies to Global, Friends, and Streamers). Friends view still uses Global ranking numbers for display. Streamers view is limited to whitelisted streamer SteamIDs plus local players, so players can compare directly to featured streamers.

Click Behavior: Global boards are the public integrity layer. Clicking a Solo row opens the standard Run Summary Page for that Top 10 run. Clicking a Duo or Trio row first opens a Player Summary Picker screen where you choose which player’s summary page to view for that co-op run. Friends boards do not expose online run summaries: clicking entries that do not belong to you does nothing (or shows a small tooltip that details are not available). Clicking your own Solo entry opens your locally saved best Run Summary Page for that board. If a friends Duo/Trio entry includes you, clicking it opens the Player Summary Picker but only your own card is selectable.

## 1.3 ONLINE SERVICES + LEADERBOARD INTEGRITY

(Defines what is stored online, how leaderboards are protected, how run review works, and how the developer moderates appeals and reports.)

### 1.3.1 SERVICE STACK + DATA OWNERSHIP

Tribulation 66 uses a split model where Steam is the public competitive layer, and a small private backend exists only to protect integrity, store reports, and enable moderation.

• Steam Leaderboards: The authoritative competitive ranking layer for Global/Weekly/All-Time boards.

• Steam Cloud: Stores save slots and run continuation data (with safe recovery behavior).

• Backend Database (Vercel + Postgres): Stores run reports, appeals, bug reports, moderation decisions, and quarantined artifacts used for integrity review.

• Sentry: Receives crash reports automatically so stability issues can be diagnosed and fixed quickly.

### 1.3.2 LEADERBOARDS (TRUSTED WRITES + SUBMISSION TIMING)

Leaderboard results are protected through a trusted-write model. Players never directly write leaderboard records. Instead, runs are validated and then submitted through a secure server-side path to Steam, preventing direct client tampering.

Submission Timing: A run does not submit continuously. Instead, it submits at major checkpoints so each segment is meaningful and comparable.

• Stage 10 / 20 / 30 / 40 / 50 / 60: Submits the cleared segment result for that difficulty checkpoint.

• Stage 66: Submits the Final segment completion result.

### 1.3.3 LEADERBOARD ELIGIBILITY (ONLINE VS OFFLINE)

Runs only count for leaderboards when the run stays eligible from start to finish. Eligibility is intentionally simple and does not rely on modded-build detection.

A run becomes NOT eligible for leaderboards only in these situations:

• The player starts the run while offline (no leaderboard submission will occur).

• The player goes offline or loses connection mid-run (the run stops being eligible immediately).

• The player turns off “Submit Scores to Leaderboard” in Settings (the run is treated as practice).

• The account is permanently banned from leaderboards due to Cheating Certainty (Forbidden Event).

When a run is not eligible, the player is still allowed to complete the run normally, but it will never submit to online boards.

Ineligible / offline runs can still unlock Achievements normally - only leaderboard submission is disabled.

Ineligible / offline runs still record full run data and event logs locally for post-run review, but nothing is uploaded.

### 1.3.4 RUN SUMMARY AVAILABILITY RULES (GLOBAL VS FRIENDS)

Run Summary Pages exist to support leaderboard integrity without turning the game into a surveillance platform. Online visibility is intentionally limited.

• Global Boards: Clicking any of the visible Top 10 rows opens the Run Summary Page for that entry, including build snapshot and event log.

• Friends Boards: Friends entries do not store or expose online run summaries. Clicking other players’ entries does not open their run details. The only exception is your own personal best entry, which is stored locally and can be inspected by you only.

• Co-op Friends Rows: A Duo/Trio friends entry is inspectable only if the local player was part of that party, and only their own local view is available. Other teammates’ build logs are not exposed.

### 1.3.5 RUN REPORTING (THIS RUN IS CHEATING)

Reporting Alignment: Player report reasons should align with automated flag categories (Score, Gold, Items, Too Lucky) to match internal review vocabulary.

For public Global Top 10 runs, viewers can flag suspicious entries directly from the Run Summary Page. Reporting is designed to collect actionable information without enabling harassment.

Reporting Rules:

• Reporter identity is always private and is never shown publicly.

• Reports are rate-limited (per-minute and per-day caps) so the system cannot be spammed.

• A report does not instantly remove or hide a run. Runs remain visible until the developer reviews and takes an explicit moderation action.

• Reports allow a short reason and an optional evidence link (example: a video timestamp). No file uploads are supported.

### 1.3.6 SUSPENSION + APPEAL REVIEW WORKFLOW

Automated Run Validation Checks (Hard Caps + Luck):

- Score Cap Check: Each stage has a maximum possible score computed from waves + enemy point values + scoring rules.

- Score-Kill Consistency Check: Verify enemies killed match score totals (prevents edited score values).

- Gold Cap Check (Non-Gambling): Validate gold totals match legitimate gold-granting events (prevents spawned gold).

- Item Provenance Check: Every owned item must map to a valid acquisition event record (Vendor purchase OR Loot Bag drop).

- Wheel Spin Count Validation: Detect abnormal counts via bounded RNG model + Luck bounds.

- Loot Bag Count Validation: Detect abnormal bag counts via bounded RNG model + Luck bounds.

- Dodge Rating Validation: Track confirmed dodge events (including touch damage avoided during dodge i-frames), dodge timing quality, and timing consistency. Flag “Dodging too perfect” when dodge quality and/or timing regularity is not human-plausible.

- Too Lucky Flag: If Luck Rating > 100, run is flagged for review.

When a run triggers strong suspicion flags, the player’s account can be marked under suspicion and leaderboard submissions can be restricted. Players are informed clearly and are allowed to appeal directly in-game.

• If the account is flagged, the Main Menu shows an Account Status button leading to the Suspension / Appeal panel.

• The player can open the Run In Question and see the same Run Summary Page layout used elsewhere (build snapshot + event log).

• Appeals are reviewed by the developer, and the review status is shown as: Appeal not submitted / Appeal under review / Appeal denied.

• If an appeal is denied, the account remains restricted and will not submit leaderboard-eligible runs until the developer reverses the decision.

Cheating Certainty (Forbidden Events): Tribulation 66 uses two disqualification types. Cheating Suspicion comes from extreme-but-possible outcomes (Too Lucky / Dodging too perfect) and allows appeal. Cheating Certainty occurs only when a forbidden event happens (an impossible event under correct rules) and has ZERO appeal.

• Cheating Certainty has ZERO appeal (no appeal button is shown).

• The player is only informed at Run End (on the Run Summary Page / Account Status Panel).

• The account is permanently locked from leaderboard participation (future runs can never appear on leaderboards).

• The exact forbidden event is shown as a clear reason string in the run review details.

Minimum Forbidden Events List (v1.0):

• Two White Loot Bags / White Items rolled back-to-back (the White rarity has a cooldown and cannot occur twice in a row).

• A duplicate White item drop occurs despite White uniqueness being ON (a White that was already obtained appears again).

• A Loot Bag produces an item that is not present in its selected rarity pool table.

• An item is acquired without a valid acquisition event record (not Vendor purchase, not Loot Bag drop).

### 1.3.7 DEVELOPER MODERATION PORTAL (TRIBULATION 66 ADMIN PORTAL)

Auto-Quarantined Runs (Reason Strings): Admin view must show FlagCategory, ReasonString, supporting numbers (score, gold, kills, luck), and which limit/bound was exceeded.

Each flagged run shows one primary reason plus any secondary supporting flags.

Moderation is handled outside the game through a private developer website called the Tribulation 66 Admin Portal. This portal is used to review reports, appeals, and quarantined runs, and to take final actions that affect leaderboard integrity.

The Admin Portal is a Vercel-hosted web dashboard accessible in a browser and protected behind an admin-only login gate. It is not accessible to players and is never shipped as part of the public game client.

Admin Portal Pages (Core):

• Appeals Queue: Review player appeals, read their message/links, and mark the appeal as approved or denied.

• Reported Runs: A queue of runs submitted by players through “This Run is Cheating.”

• Auto-Quarantined Runs: Runs quarantined automatically by integrity rules (hard illegal metrics, extreme anomalies, etc.).

• Accounts (SteamID Actions): Suspend / Unsuspend / Ban, plus internal notes.

• Bug Reports: Review player-submitted bug reports with last-run context.

• Audit Log: A chronological list of every moderation action taken (for accountability and sanity checking).

### 1.3.8 BUG REPORTING + CRASH REPORTING

Tribulation 66 supports two types of stability reporting: manual player bug reports and automatic crash reports.

• Report Bug (Manual): Players can submit a short bug report from within the game. The report includes a text description and auto-attaches lightweight run context (stage, mode, difficulty, and other relevant state). It does not support file attachments.

• Crash Reports (Automatic): If the game crashes, a crash report is sent to Sentry automatically so the issue can be investigated and fixed. This is designed to be unattended and does not require the player to do extra work.

### 1.3.9 DATA RETENTION + PRIVACY

Online storage is intentionally minimal and temporary. The goal is integrity and support, not permanent surveillance. The backend automatically expires old entries so the database stays small and private.

Typical retention windows:

• Bug reports expire after roughly 30 days.

• Appeals expire after roughly 90 days post-resolution.

• Quarantined artifacts expire after roughly 180 days.

• Public Top 10 run details persist while the run remains in Top 10 (plus a short grace window).

## 1.4 ACCOUNT STATUS PANEL (SUSPENSION / APPEAL)

Example categories: Score above limit; Score mismatch to kills; Gold above limit; Items not from valid sources; Too Lucky; Dodging too perfect.

Example categories: Score above limit; Score mismatch to kills; Gold above limit; Items not from valid sources; Too Lucky.

(Opens when you press Account Status on the Main Menu. The button is visible when your account has a run flagged under Cheating Suspicion OR disqualified under Cheating Certainty.)

The Account Status Panel is a large modal overlay that opens on top of the Main Menu. The background is dimmed, and the player’s focus is pulled to a wide, two-column panel that clearly explains why the account is restricted and how to appeal.

Left Region (Suspicion Details): The left half starts with a large warning headline in bold red text reading: RUN UNDER SUSPICION OF CHEATING. Directly below, the panel lists the reason the run was flagged. These reasons are based on anti-cheat flags such as: “Too Lucky”, “Dodging too perfect”, or “Items spawning unexpectedly.” The reason text is readable, blunt, and clearly communicates what the system detected.

Below the reasons is a single button labeled RUN IN QUESTION. Clicking this button opens the standard Run Summary Page for the exact run that triggered the suspicion. This is the same run summary layout the player would normally see after finishing a run, including build details and the run log view.

At the very bottom of the left region is a Back button that closes the Account Status Panel and returns to the Main Menu.

Right Region (Appeal Box - Suspicion Only): The right half contains a large text input field where the player can write an appeal message and optionally paste an evidence link (example: a YouTube link). Below the input box is a Submit Appeal button.

Cheating Certainty Rule: If the restriction reason is a Forbidden Event (Cheating Certainty), the Appeal Box is replaced with a clear locked message stating: NO APPEAL AVAILABLE. The player can still view the Run In Question details.

## 1.5 PLAYER SUMMARY PICKER (CO-OP LEADERBOARD RUN)

(Opens after clicking a Duo/Trio leaderboard row. You choose which teammate’s summary page to view.)

The Player Summary Picker is a short selection screen that only appears when the player clicks a co-op leaderboard entry (Duos or Trios). It is styled like the Party Size Picker: large, clean choice panels that are immediately understandable.

Across the center of the screen are either two large panels (for Duos) or three large panels (for Trios). Each panel is a big clickable card. Inside each card is the player’s name in large text, and the card background contains a large hero image representing the hero that player used in the run. The hero image can be a portrait or a cropped character render that fills most of the card.

Clicking a player panel opens the Run Summary Page for that run, focused on the chosen player’s build and run details. If this screen was opened from a Global Top 10 leaderboard row, any player panel can be selected. If opened from a Friends leaderboard row that contains the local player, only the local player’s panel is selectable (others are disabled), since friends run summaries are not stored online. A Back button returns to the previous leaderboard view.

## 1.6 QUIT CONFIRMATION PANEL

(Opens when you press the Quit button in the top-right of the Main Menu.)

When the player presses Quit on the Main Menu, a small confirmation panel appears centered on the screen. The Main Menu remains visible behind it but becomes slightly dimmed so it is clear the menu is temporarily not interactive.

The panel contains a short title and message asking if the player is sure. Under the text are two large buttons: a safe button (No, I want to stay) and a confirm button (Yes, I want to quit).

Pressing the safe button closes the panel instantly and returns the player to the Main Menu. Pressing the quit button closes the application.

## 1.7 LANGUAGE SELECT PANEL

(Opens when you press the Language icon button in the bottom-left of the Main Menu.)

The Language Select Panel is a centered modal overlay that lists all supported languages in a clean vertical list. The current screen remains visible behind it, slightly dimmed.

Each language appears as a full-width row that can be highlighted on hover. Selecting a language instantly previews the change wherever possible, so the player can immediately see menu text update.

At the bottom are two buttons: Confirm and Back. Confirm applies the selected language and closes the panel. Back closes the panel without changing the language.

## 1.8 PARTY SIZE PICKER

(Opens after selecting New Game or Load Game from the Main Menu.)

The Party Size Picker is a simple choice screen with three large centered panels: Solo, Duo, and Trio. The panels are aligned horizontally across the middle of the screen and are large enough to feel like big “cards” rather than small buttons.

Each card includes a bold label and a simple icon indicating the number of players (one silhouette for Solo, two for Duo, three for Trio). Hovering highlights a card. Clicking a card confirms the party size choice.

A Back button sits near the bottom-left so the player can return to the Main Menu.

## 1.9 SAVE SLOTS (LOAD GAME)

(Opens after Party Size Picker when you choose Load Game for the selected party size.)

The Save Slots screen shows a clean grid of saved runs. The main area is a 3x3 grid of large slot tiles (nine slots per page). Each tile looks like a card with a title, a small run snapshot, and a stage indicator.

Occupied slots look filled and clickable. They show details like the current stage number, the last played timestamp, and a small party roster indicator if the save is co-op. Empty slots are visible but appear disabled and cannot be selected.

Save Slot Resilience: Each save slot maintains a small rolling backup history so a run can be recovered if a write is interrupted or a file becomes corrupted. Every slot keeps the current save plus two automatic backups. When loading, the game always prefers the newest valid save, but if a conflict or corruption is detected, a safe recovered copy is preserved rather than forcing the player to lose progress.

At the bottom center are paging controls: Prev Page, a “Page X/Y” indicator, and Next Page. A Back button returns to the Party Size Picker.

## 1.10 HERO SELECTION (SOLO)

(Opens after Party Size Picker -> Solo (New Game), or after loading a Solo save.)

The Solo Hero Selection screen is a full-screen hero showcase built around a central 3D character preview. The overall layout has three main areas: a top hero belt carousel, a left skins panel, and a right hero information panel.

Top Hero Belt + Hero Grid Button: Along the very top is a curved “hero belt” carousel of hero icons, where the centered hero icon is larger and the icons recede slightly toward the sides. Near the top-left, around the beginning of the belt, there is a button labeled HERO GRID. Pressing this opens a large modal that shows all heroes in a fast grid layout (Dota-like) for quick selection.

Center Preview: In the middle lower area is a large 3D hero model standing idle, with the currently selected companion posed beside the hero like a collectible partner. The player can rotate the main preview for a full 360-degree view. A floating button labeled CHOOSE COMPANION sits above the companion’s head area, making it visually obvious that this button changes the companion selection.

Body Type Toggle: Every Hero has two body types. Body Type A is the hyper-masculine, huge bulging muscles model. Body Type B is the same hero as a super-model looking woman. The toggle lives in the selection UI so the player locks their preferred body type before entering the run.

Left Panel (Skins): The left side of the screen contains a tall skins panel. Each skin is displayed as a horizontal row. If the skin is not owned, the row shows a preview and a BUY button. If the skin is owned, the row shows an EQUIP button and a SELL button. Selling returns the coins spent on that skin. The currently equipped skin is clearly marked.

Map Theme: In Solo, the map theme is driven by the chosen Hero. In Co-op, the run alternates between the map themes of all players in the lobby. Skins do not override the theme; they are cosmetic identity inside the chosen theme. Rotation order is locked to Host → Player 2 → Player 3 → repeat.

Right Panel (Hero Info): The right side contains the hero info panel. At the top is the HERO NAME in large text. Below is a rectangular video window showing a short looping gameplay clip of the hero in action. Beneath the video is a single-line description summarizing the hero’s playstyle. Below that are four achievement medals (Black, Red, Yellow, White) displayed as icons. They are grayed out until unlocked and are purely cosmetic achievements with no gameplay effect.

On the edge of the hero info panel is a small book-shaped icon button. Pressing it opens a Lore modal that displays the hero’s lore text on a dedicated overlay.

Difficulty Selection: Instead of choosing a starting chapter, the hero selection includes a difficulty selector that matches the leaderboard difficulties: Easy, Medium, Hard, Very Hard, Impossible, Perdition, Final. The selected difficulty is clearly highlighted.

Catch-Up Stage (Starting Above Easy): If the player starts a New Game on a higher difficulty, they first enter a short catch-up stage with a Mega Piñata in the center. The Mega Piñata scales in size based on the starting difficulty (harder start = bigger piñata). Interacting with it awards Gold only (no items). For items, a one-hit enemy called Present Man appears (a stack of wrapped presents whose size also scales with difficulty). Present Man dies in one blow and drops all presents at once (a burst of Loot Bags / item rolls). Overall reward target remains roughly equivalent to playing through Stages 1-10.

Navigation: A Back button returns to the Party Size Picker. A THE LAB button opens the Practice Lab (testing room) using your currently selected hero + companion loadout, and exiting the Lab returns you back to this Hero Selection screen. A large ENTER THE TRIBULATION button starts the run.

## 1.11 HERO GRID (MODAL)

(Opens when you press HERO GRID on the Hero Selection screen (Solo or Co-op).)

The Hero Grid modal is a large overlay that covers most of the screen, designed for fast hero selection. It is inspired by Dota-style hero grids: many hero portraits visible at once with clear grouping and fast scanning.

The modal background is slightly transparent with the Hero Selection screen dimmed behind it. At the top of the modal is a small header bar with the title HERO GRID and simple sorting or filtering controls (for example, by role or alphabetical).

The main body of the modal is a dense grid of hero portrait tiles. Each tile shows a hero face/portrait and highlights on hover. Clicking a portrait instantly selects that hero and closes the modal, returning the player to the main Hero Selection preview for that hero. A close or Back button dismisses the modal without changing selection.

## 1.12 HERO LORE (MODAL)

(Opens when you press the book icon on the hero info panel.)

The Hero Lore modal is a clean reading overlay that focuses entirely on story text. The hero selection screen dims behind it, and a centered panel shows the hero’s name at the top with a scrollable lore text body beneath.

The lore text is formatted for readability with comfortable spacing. A Back button closes the modal and returns the player to Hero Selection.

## 1.13 COMPANION SELECTION

Companion Optionality: The player can choose a companion or explicitly select NO COMPANION. Selecting no companion is a deliberate and valid run choice.

Companion Body Types: Companions also have Body Type A / B. For companions, Body Type A is the standard woman form. Body Type B is a cutesy pet form of the companion’s original demon form.

Companion Lore Format Rule: Every companion lore entry begins with the phrase “She tells me…” followed by her story.

Endgame Betrayal Twist: If the player is about to beat the final boss while a companion is equipped, she reveals she was a demon all along. She transforms into her original demon form and kills the player in the last moment, causing the run to fail. This is why selecting NO COMPANION must always remain an option.

(Opens when you press CHOOSE COMPANION above the hero preview.)

The Companion Selection screen mirrors the layout of Hero Selection so it feels consistent. There is a top companion belt carousel, a left skins panel, a central 3D companion preview, and a right companion information panel.

Top Companion Belt + Grid Button: Across the very top is a curved belt of companion icons. Near the top-left is a COMPANION GRID button that opens a large grid modal for fast selection (same concept as the hero grid).

Center Preview: The middle area shows a large 3D companion model that can be rotated 360 degrees. The companion stands in an idle pose, clearly visible and framed like a collectible showcase.

Left Panel (Skins): The left side contains a skins panel with rows. Unowned skins show a preview and BUY button. Owned skins show EQUIP and SELL, where selling refunds the coins spent. The equipped skin is clearly highlighted.

Right Panel (Companion Info): The right panel contains the companion’s name, a short lore/description area, and the universal passive effect the companion provides. All companions share the same passive effect: healing the player in-game.

Companion Medals (Cosmetic Only): Each companion has 3 medals that track accomplishments only and do not change gameplay. The medals represent clearing Easy, Medium, and Hard difficulty segments while that companion is equipped. These medals are purely for collection/completion status.

A small book-shaped icon button sits on the edge of the panel to open the Companion Lore modal. At the bottom of the screen, where the Enter the Tribulation button would normally be, there is a large CONFIRM COMPANION button that accepts the selection and returns you back to Hero Selection.

## 1.14 COMPANION LORE (MODAL)

(Opens when you press the book icon on the companion info panel.)

The Companion Lore modal is a reading overlay just like the Hero Lore modal. The companion selection screen dims behind it, and a centered panel shows the companion’s name at the top with scrollable lore text beneath.

A Back button closes the lore modal and returns to Companion Selection.

## 1.15 HERO SELECTION (CO-OP)

(Used for Duo/Trio runs. Same overall structure as Solo Hero Selection, with co-op identity shown.)

Co-op Hero Selection uses the same layout and interactions as Solo Hero Selection: the hero belt at the top, the HERO GRID button for fast selection, the central rotatable 3D preview, skins on the left, and hero info on the right.

The primary difference is that the screen clearly indicates this is a co-op party. A small party roster area shows the teammate slots (two slots for Duos, three slots for Trios). Each slot displays the player’s name and either their selected hero portrait or a “Selecting…” placeholder while they choose.

Difficulty selection is present here as well and matches the same list used by leaderboards (Easy through Final). A THE LAB button is also available for practice/testing: it loads the Practice Lab using the party’s current selections, and exiting returns the player to this Co-op Hero Selection screen. When the party is ready, the host starts the run with ENTER THE TRIBULATION, while non-host players may show a Ready button or simply wait for the host to begin.

## 1.16 THE LAB (PRACTICE / TRAINING ROOM)

(A dedicated practice-only testing room where the player can freely spawn enemies/NPCs and grant themselves items to test builds and interactions. Always zero rewards.)

Purpose: The Lab is a training room meant for experimenting with builds, damage, and system interactions without committing to a full run. The Lab always counts as Practice and grants no rewards of any kind.

Environment: The Lab uses a clean, minimal training arena (white grid-room style) so combat and effects are easy to see and debug.

Loadout Rules: The Lab always loads your currently selected Hero, equipped Hero Skin, Companion, and equipped Companion Skin.

Practice Rules (always enforced):

• No achievements

• No cosmetic unlocks

• No leaderboard submission

• No run saves

Access: The Lab is accessed from Hero Selection (Solo or Co-op) via a The Lab button. Exiting The Lab always returns you back to the same Hero Selection screen that launched it. If entered from a co-op lobby, the party is preserved.

Co-op Ready Check Behavior: If a player is inside The Lab during co-op setup, the host can still initiate a Ready Check and the Ready / Not Ready overlay still appears inside The Lab so the player can respond.

Unlock Rules (What appears in the Lab lists):

• Items appear only after you have obtained that item at least once in any run type (Trusted / Untrusted / Practice).

• Enemies appear only after you have killed that enemy at least once.

• Locked entries are hidden completely (only unlocked content appears).

• When something becomes Lab-unlocked, a toast notification appears, and toasts are queued so only one shows at a time.

UI Layout: The Lab UI is built around two minimizable side panels + an Exit action.

Left Panel — Items (Minimizable):

• Shows a list/grid of unlocked items.

• Selecting an item grants it directly to your inventory.

• Reset Items removes all items granted during the current Lab session (returns you to baseline).

Right Panel — Enemies (Minimizable):

• Spawn list is organized into tabs: NPC, Mobs, Stage Bosses.

• Select an entity and press Spawn to create it (one spawn per press; press repeatedly for multiples).

• Reset Enemies destroys all currently spawned entities.

• Tree of Life is spawned from the Enemies panel under NPC (treated as an NPC/interactable).

Exit: Exit The Lab returns you to the Hero Selection screen that launched it (Solo or Co-op).

## 1.17 SETTINGS

(Opens from the Main Menu or from the in-run Pause Menu. Centered modal panel.)

Settings is not a full-screen page. It is a centered modal panel that takes up most of the screen, with the game or menu still visible behind it. The background scene remains visible but dimmed so the player understands they are inside a configuration layer.

Settings can be closed at any time via the X button in the top-right corner, by pressing Esc on keyboard, or by pressing Back on controller. Closing always exits immediately - there is no global Cancel button.

Across the top of the panel are five tabs in this exact order: Gameplay, Graphics, Controls, Audio, Crashing. Settings remembers the last-open tab per player, so returning to Settings brings you back to the tab you were last using.

Gameplay, Controls, and Audio changes apply immediately (including mid-run). Graphics changes are staged and only apply when pressing Apply inside the Graphics tab. Closing Settings discards any unapplied Graphics changes.

Gameplay Tab (Modes + Preferences): All options in this tab are local per player. In co-op, nothing in this tab forces teammates to use the same settings, and mixed settings are allowed inside a run. This tab can be changed mid-run and takes effect instantly.

The Gameplay tab contains the following options: Intense Visuals (toggle), Auto Sprint (toggle), and other run preferences. Hitbox visualization is not configured as a Settings toggle (see Gamer Mode keybind).

Gamer Mode (Hitboxes) - Keybind Only: Gamer Mode shows hitboxes and projectile hitboxes and is toggled only through a dedicated keybind (not a Settings option). This keybind unlocks only after clearing Stage 50. Gamer Mode does not change gameplay outcomes; it is a visibility/debug overlay only.

Graphics Tab (Apply-Only): The Graphics tab contains monitor selection, resolution selection, window mode selection (Windowed / Fullscreen / Borderless Windowed), and a display mode selector (Standard / Widescreen). It also contains a 4-notch Quality slider labeled Best Performance on the left and Best Quality on the right, with curated presets only (no per-setting mix-and-match).

Graphics also includes an FPS Cap selector with the options 30, 60, 90, 120, Unlimited. Menus are internally capped to 60 FPS; the FPS cap applies only during runs. Unlimited is allowed only for in-run gameplay.

The Apply button only appears on the Graphics tab. After applying Resolution or Display Mode changes, the player is shown a “Keep these settings?” safety prompt with a 10-second auto-revert timer.

Controls Tab (Full Rebinding): The Controls tab supports full rebinding for Keyboard & Mouse and for Controller. It is split into two sub-tabs (Keyboard & Mouse, Controller). Each action supports Primary and Secondary bindings, conflicts overwrite with a warning, and each bind row includes a Clear button. Restore Defaults exists separately for Keyboard & Mouse and for Controller, and each Restore Defaults action requires a confirmation popup.

The Controls tab also includes a dedicated binding for toggling the Media Viewer panel (TikTok / YouTube Shorts viewer) during a run.

### 1.17.1 CONTROLS + INPUT LAYER (LOCKED)

Tribulation 66 uses a small, readable control set designed for fast movement, instant decisions, and minimal finger gymnastics. The player is never fighting their keyboard/controller - the controls are intentionally compact.

Auto Attacks are automatic - the player does not shoot manually as a primary input. Attacking is proximity-driven, with optional manual focus via Attack Lock. Ultimates ignore Attack Lock.

Manual Attack Lock (KBM baseline):

• If the player left-clicks while the mouse is over an enemy, the hero locks onto that enemy.

• Clicking again toggles it off, or clicking a different enemy swaps the lock.

• Left-clicking empty space does nothing.

Full Map: The full-screen map is opened with a dedicated Map input and fills the screen as a focused overlay.

Interact: Interact is used for all world interactables, dialogue NPCs, and pick-up interactions. In co-op, reviving a teammate is a single Interact press (not a hold).

### 1.17.2 DEFAULT CONTROLS (v1.0)

Keyboard & Mouse (Default):

Movement

• Move Forward: W

• Move Back: S

• Move Left: A

• Move Right: D

• Dash / Evade: Space (Secondary: Left Shift)

Combat

• Ultimate: Right Mouse Button (Secondary: Q)

• Manual Attack Lock: Left Mouse Button (hover enemy -> click to lock)

Interaction

• Interact: E (Co-op Revive = Tap E on teammate; plays defib revive animation with i-frames)

Run Overlays

• Open Full Map: M

• Toggle HUD Panels (toggleable panels set): Tab

• Toggle Media Viewer Panel: V

• Toggle Gamer Mode (Hitboxes + Projectile Hitboxes) (Stage 50 Unlock): G

Pause / Menus

• Pause Menu: Esc

• Restart Run (Instant) (Host-only in co-op): R

• Confirm / Select (UI): Enter (Secondary: Left Click)

• Back / Cancel (UI): Esc

• Tab Left / Right (UI Tabs): Left Arrow / Right Arrow

Controller (Default) (Xbox-style naming):

Movement

• Move: Left Stick

• Look / Aim Cursor: Right Stick

• Dash / Evade: A

Combat

• Ultimate: Y

• Manual Attack Lock: Right Stick Click (R3) - Locks the most centered enemy on screen (or the nearest enemy if none is centered). Press again to clear lock.

Interaction

• Interact: X (Co-op Revive = Tap X on teammate; plays defib revive animation with i-frames)

Run Overlays

• Open Full Map: View / Back button

• Toggle HUD Panels (toggleable panels set): D-Pad Up

• Toggle Media Viewer Panel: D-Pad Down

• Toggle Gamer Mode (Hitboxes + Projectile Hitboxes) (Stage 50 Unlock): Unbound (rebindable)

Pause / Menus

• Restart Run (Instant) (Host-only in co-op): Unbound (rebindable)

• Pause Menu: Menu / Start

• Confirm / Select (UI): A

• Back / Cancel (UI): B

• Tab Left / Right (UI Tabs): LB / RB

### 1.17.3 INPUT PRIORITY RULES (LOCKED)

Gameplay State (Normal Run):

• Movement + Dash + Interact + Ultimate are active.

• Auto Attacks happen automatically as designed.

Paused State (Pause Menu):

• Gameplay is frozen.

• Only UI navigation inputs work.

• In co-op: if any player pauses, it pauses for everyone, and only the player who paused can unpause.

Non-Pausing UI Overlays (Vendor / Gambler / Boss Altar / Wheel Popup):

• Stage time continues while the overlay is open.

• Only UI inputs are accepted while the overlay is open.

• Combat inputs are suppressed.

Back Button Rule (Always consistent):

• Back closes the most recent/top overlay first.

• If nothing is open, Back opens Pause Menu.

Audio Tab: Subtitles are always enabled (no toggle). Audio contains three sliders (Master, Music, SFX), a Mute when unfocused toggle, and an Output Device selector. Audio changes apply immediately. UI Feedback: Clicking any UI button plays a click sound. Dragging or adjusting any slider plays a sliding/tick sound.

## 1.18 AUDIO SYSTEM (MUSIC + SFX)

UI SFX: All UI buttons/toggles/sliders play appropriate feedback sounds.

Main Menu Music: Main Menu lobby music plays while browsing the main menu.

Hero Carousel Music: Previewing a hero plays that hero’s OST; hero OST has variants by heart count.

Boss Music: Each stage boss has unique OST with health-based variations; bosses get mechanically harder as HP drops.

Flow State Music Override: When Flow State (Ultra Instinct) triggers, the game swaps to a unique Flow State music layer for that hero, overriding stage music until Flow State ends.

Vendor/Gambler Music: Vendor/Gambler bubble music has variants by anger meter intensity, and each has a separate boss OST.

Saint Music: Saint has bubble OST only.

Ouroboros Music: Ouroboros has proximity warning OST near the danger ring plus boss OST.

Loan Shark Music: As the Loan Shark approaches, a warning OST fades in and ramps dynamically by distance/intensity. Loan Shark music overrides all other music while active.

(Defines how music and sound behave across menus, stages, bosses, and special moments. This is a gameplay-facing system description, not an engine implementation.)

Tribulation 66 uses a state-based audio system so the player always hears the correct mood for what is happening, without abrupt or confusing transitions. Music is designed to be readable and memorable, while sound effects emphasize combat clarity, pickup feedback, and moment-to-moment danger.

Music States (What plays when):

• Main Menu + Frontend screens use a stable menu theme that feels ominous and high-stakes but not exhausting.

• In-Run Stage Combat uses a looping combat track for the current stage theme.

• Boss Fight overrides stage combat music with a dedicated boss track the moment the boss encounter begins (entering Boss Quarters / boss becomes active).

• World Interactable Houses temporarily override the stage combat track while the player is inside their safe bubble: Black House (Vendor) uses a grimy old-salon shop vibe, Yellow House (Gambler) uses a flashy high-risk casino vibe, Red House (Ouroboros) uses a menacing danger vibe, and White House (Saint) uses a church-like sacred vibe.

Music Transition Rules:

• Entering a boss fight crossfades into the boss track with a short stinger (a sharp musical hit) to signal escalation.

• Exiting a world interactable crossfades back into the stage combat track at a sensible musical boundary so it feels smooth, not random.

• When the stage timer reaches the miasma pressure point (late-stage), the music intensifies (either by swapping to a higher-energy variant or layering additional percussion) to communicate urgency.

Core Sound Effects Philosophy:

• Combat SFX prioritize readability: enemy attacks, incoming danger, and successful hits must be immediately recognizable even in chaos.

• UI feedback is consistent: clicking any UI button produces a clean click, and sliders produce a subtle tick/slide sound.

• Major events have audio callouts (short stingers) such as: Run Start, Wave Cleared, Boss Fight Start, Boss Defeated, and Run End.

Ult Voice Line + Subtitles (Locked): Activating any hero Ult plays a unique English voice line for that hero plus a special Ult activation stinger. The voice line always displays subtitles using localized strings for the currently selected language.

Media Viewer Audio Rule: When the TikTok / YouTube Shorts Media Viewer is open, all other game audio is muted. When the viewer closes, the game restores audio exactly to the player’s previous slider values (no resets).

Crashing Tab: This tab is a clear emergency checklist for stability. It explains exactly what to do when the game is crashing and includes safe “stability-first” settings presets (for example: lower quality preset, reduce intense visual effects, disable extra particles, and simplify optional overlays). The goal is that a player can recover from crashes without guessing or needing external support.

Report Bug + Crash Reports: The Crashing tab also includes a Report Bug button that lets players submit a short text description (links allowed). The report automatically includes lightweight context about the player’s last run and the current build so issues can be reproduced. Bug reports are text-only and never block gameplay. Separately, if the game crashes, a crash report is captured and uploaded to Sentry automatically for investigation.

Media Viewer Audio Rule: Opening the TikTok / YouTube Shorts viewer temporarily disables all other game audio. When the viewer is closed, audio returns exactly to the player’s previous slider values and toggles (it does not reset).

## 1.19 ACHIEVEMENTS

(Opens when you press Achievements on the Main Menu.)

The Achievements screen is a full-page achievement browser with a clean header and a large list of horizontal achievement rows. The top-left shows the title ACHIEVEMENTS, while the top-right shows the player’s total Achievement Coins, displayed with a coin icon and a large number.

Below the header is a row of four category toggles that act like tabs: Black, Red, Yellow, White. These represent achievement rarity tiers.

The main content area below the tabs is a vertical list of horizontal achievement rows. Each row is a single long strip. The layout has three content zones, but the column borders are not visible: the challenge description sits on the far left of the row, the progress sits in the middle (either as text like “3/10” or a small progress bar), and the reward sits on the far right as a coin icon plus the amount of Achievement Coins. The row styling stays clean and minimal with no visible grid lines.

# 2. FRONTEND - IN RUN

## 2.1 RUN LOADING SCREEN

(Appears when a run is starting or resuming. Full-screen lore loading splash.)

The Run Loading Screen is a short but flavorful transition into gameplay. Instead of a generic loading screen, it shows a full-screen lore image focused on either the selected Hero or the selected Companion, portraying their life before the Tribulation.

This loading splash is designed to build personality and story context while the run loads. The screen is clean and cinematic, with a subtle loading indicator and minimal text so it stays fast and never feels like a separate “menu screen.”

## 2.2 START AREA + GATE START

All stages begin in a Start Area: a small, safe staging zone that exists in every stage before the main combat space. The stage countdown timer and wave pressure do NOT begin until the player exits the Start Area through the gate. (Speedrun time begins immediately when the player spawns.)

New Game Arrival (Meteor Drop): When entering a run for the first time (or starting a fresh run), the hero drops in via a Meteor Landing into the Start Area. The landing is dramatic but brief, and camera control resumes immediately after the landing completes.

Restart Entry (Fast): If the run is restarted (Pause Menu → Restart), the Start Area entry is fast and skips the Meteor Landing. Resuming a saved run also skips the Meteor Landing.

First-Run Tutorial Behavior (Stage 1 Only): If this is the player’s first time ever playing, Stage 1 uses a special larger Tutorial Start Area. This larger Start Area contains the full tutorial flow and guidance before the player exits into the main map. In all other cases (including all later stages), the Start Area is the standard small version.

Tutorial White Item Demonstration: During the first-run tutorial, a single safe tutorial enemy spawns and drops a guaranteed White item bag. The tutorial also forces one White proc by requiring one Auto Attack hit so the mechanic is clearly demonstrated.

Gate Start Rule: After the tutorial sequence (first run) or immediately (all other runs), the player exits the Start Area through the gate into the stage map. Crossing this gate begins the stage countdown timer and wave system for that stage.

## 2.3 MAP (PROCEDURAL STAGE LAYOUT)

Each stage is procedurally generated using a consistent layout rule set. Every stage contains a Start Area (safe staging zone) and one or more combat spaces depending on the stage type. Placement rules prevent landmarks from stacking on the player or clustering unnaturally.

Map Theme: In Solo, the map theme is driven by the chosen Hero. In Co-op, the run alternates between the map themes of all players in the lobby so every player’s identity is represented across the run. Rotation order is locked to Host → Player 2 → Player 3 → repeat.

Boss Encounter Trigger (Locked): There is no Summoning Stone. Stage bosses spawn into their arena when the map is generated and are already present in their area from the start (visible from a distance). The boss fight begins when the player enters Boss Quarters and the boss becomes active.

Normal Stage Layout (Not ending in 5 or 0): Start Area → Central Map (main arena + corner houses) → Boss Quarters (boss already present). Entering Boss Quarters activates the boss fight; the Trickster can appear with a short animation and brings the Cowardice Gate.

Corner Landmarks (4 Houses): Normal stages contain the four corner houses (Vendor, Gambler, Ouroboros, Saint) placed in the combat map. Boss-only checkpoint stages (stages ending in 5 or 0) do NOT include the four corner houses and are only Start Area + Boss Quarters.

Indestructible Geometry: The major map layout (buildings, ridges, rivers, and landmark structures) is indestructible so navigation and sightlines stay stable. No destructible buildings or objects exist in the game. All props and set-dressing remain intact at all times.

## 2.4 IN-RUN HUD

(The full gameplay HUD visible while fighting through stages.)

The in-run HUD is deliberately split into two categories: Ever-Present elements that are always visible and never toggled off, and Toggleable Panels that the player can show or hide with a dedicated HUD toggle input. The goal is to keep combat readability high while still giving the player access to deep run data whenever they want it.

Ever-Present Anchor (Bottom-Left): The hero portrait is presented as a wanted poster. The poster includes the word WANTED, and the Bounty value displayed on the poster is the player’s current Score. The hero portrait itself is a sprite that changes expression based on the player’s current hearts. At 1 heart the face looks barely surviving, at 2–3 hearts it looks really beat up, at 4–5 it looks upset, at 6 it looks neutral, at 7–8 it looks happy, at 9 it looks very happy, and at 10+ it looks ecstatic.

Hearts + Skulls (Above Portrait): The health display shows a maximum of 5 heart icons above the hero portrait. If the hero has more than 5 hearts, the row remains at 5 icons and the hearts change color using the Heart Color Progression system to represent the higher-tier heart count. In co-op, this Heart HUD block also shows the player’s name above their hearts. If a teammate is dead, their map marker is shown as DEAD.

Skull Increase Rule (locked): Skulls increase ONLY by activating Difficulty Totems found around the stage. Totems apply instantly and globally to the entire party. Totem skull values: Black +0.5 skull, Red +1 skull, Yellow +3 skull, White +5 skull. Half-skulls are displayed visually.

Status-to-Hearts Visual Rule: If the hero is affected by a status effect applied by a Unique enemy, the heart icons visually change to match the status (burning, freezing, cursed, etc.). The effect persists for the duration of the status effect.

Hero Ult Cooldown Icon (Above Hearts): Each hero has a single Ultimate ability. The Ult is shown as a small icon above the hearts. When available, the icon is fully lit. When used, it greys out and displays a countdown timer inside the icon until it recharges. Tuning target: Ult is usable roughly once per stage (~6 minutes), exact cooldown values tunable.

Heat Bar (Above Portrait / Below Hearts): A Heat bar sits above the hero portrait and below the hearts. It is always visible. Heat increases from damage taken and from successful dodges (including touch damage avoided during dodge i-frames). Heat persists across stages and resets when a full difficulty segment is completed. Heat is used to trigger Flow State (Ultra Instinct) when the player would otherwise die on their last heart during specific boss fights (see In-Run Stage Flow).

Flow State HUD Overrides: When Flow State triggers, the hero portrait swaps to a unique Flow portrait sprite and the HUD displays a visible 30-second Flow timer near the Heat bar until the state resolves.

Idol Slots (Sides): Each hero has exactly 3 Idol slots total (always available; no unlock schedule).

Stage Timer (Top-Center): The stage countdown timer begins only after the party exits the Start Area. Once started, it follows the normal rules (miasma rise late-stage and full coverage at timer end). Speedrun time tracking begins immediately on spawn.

Wallet + Combat Readouts: The player’s wallet is shown near the wanted poster anchor so purchases and gambling decisions feel immediate. A Debt icon appears above the Gold amount so the player always sees what they owe. During boss encounters, the boss uses a large Elden Ring style health bar with segmented colored tiers.

Boss HP Tier Colors (by difficulty segment):

• Easy (Stages 1-10): Red

• Medium (Stages 11-20): Purple -> Red

• Hard (Stages 21-30): Blue -> Purple -> Red

• Very Hard (Stages 31-40): Green -> Blue -> Purple -> Red

• Impossible (Stages 41-50): Yellow -> Green -> Blue -> Purple -> Red

• Perdition (Stages 51-60): Orange -> Yellow -> Green -> Blue -> Purple -> Red

• Final (Stages 61-66): White -> Orange -> Yellow -> Green -> Blue -> Purple -> Red

Toggleable Panels (Fixed Positions): Several informational panels can be shown or hidden. These panels are not draggable. They have fixed default positions and fixed sizes, except for the Mini Map and Media Viewer which support a small set of size presets. Each panel includes a Toggle Lock button that prevents it from being affected by the global HUD toggle input.

Toggleable panels include: Mini Map (top-right), Speedrun Timer (top-right, left of the Mini Map - only when Speed Runner Mode is enabled), Stats Table (right side below the Mini Map), Inventory Panel (bottom area to the right of the hero portrait), and the Media Viewer panel (top-left). The Media Viewer is a native-feeling in-game TikTok and YouTube Shorts viewer running through Unreal Chromium and does not require Steam Overlay or opening external browser windows.

Speedrun Pace Overlay: When Speed Runner Mode is enabled, the HUD shows your current pace per stage compared against the pace of Rank #10 on the Weekly leaderboard for your selected difficulty. This overlay is powered by benchmark split timing (stage-by-stage timestamps), not only the final leaderboard time, so it can show whether you are ahead or behind pace while the run is still in progress. The comparison target automatically upgrades to Rank #9, #8, etc. if you are currently faster than those paces.

Benchmark Split Timing Rules: The benchmark run provides a timeline of when it reached each stage (Stage 1, Stage 2, Stage 3, etc.). These splits are derived from the benchmark run’s stored event-log timing data and cached locally at run start so the overlay updates smoothly during gameplay. If the run becomes offline or the player toggles off Submit Scores to Leaderboard, the overlay can remain visible, but it no longer represents leaderboard-eligible pacing.

Full Map: The Mini Map is a small corner panel, but the full game map is opened with a dedicated Map input (M) and fills the screen as a focused overlay view.

## 2.5 HEROES (ROSTER)

Hero Class Identity Rule (Design Direction): Each hero must feel like a distinct RPG class/playstyle (example: Berserker, Tank, Elusive Samurai, etc.).

This identity is achieved through three pillars (locked):

• Unique Auto Attack identity (projectile behavior, pacing, range, and feel).

• Unique Ult identity (one button, but visually and mechanically distinct per hero).

• Base stat biases and scaling interactions (so Power gains feel different per hero even though Power always increases Damage + Attack Speed + Scale).

Auto Attack Targeting Rules:

• Auto Attacks fire automatically when enemies are within range (proximity-driven).

• Default targeting selects the closest enemy in range.

• Manual Attack Lock: If the player left-clicks while the mouse is over an enemy, the hero locks onto that enemy and focuses it while it is in range.

• Lock breaks when the locked enemy dies, when the player clicks the same enemy again (toggle off), or when the player locks a different enemy.

• If the locked enemy is out of range, the lock indicator remains visible but the hero attacks the closest enemy in range until the locked enemy returns into range.

• Left-clicking empty space does nothing (no movement command and no lock clear).

• Attack Lock affects Auto Attacks only. Ultimates ignore Attack Lock and behave normally.

Hero Base Stats (Required): Define a single generic base stat list for all heroes so every item/idol modifier has a base value to modify (values tuned later).

Damage Model (Sources vs Modifiers):

- Damage Sources: Auto Attack + White Items only.

- White Items: Grant the MOST Power and also trigger a White Proc effect (true damage source). Dropped from Loot Bags only. White items cannot duplicate (uniqueness ON).

- White Item boss damage is split evenly across the White Item’s hit count (example: 10 slashes = 2.5% each).

- All other items + idols modify Auto Attack (AOE, scaling, DOT like poison) and do not count as separate damage sources.

(Hero kits are defined by auto-attack projectile identity and one Ult button. Heroes do not have separate passives.)

Body Types: Every hero supports Body Type A (hyper-masculine, huge bulging muscles) and Body Type B (super-model looking woman). Body type is chosen in the selection UI and is cosmetic identity only.

- Example: Alice in Wonderland Rabbit — Auto: Spinning pocket-watch gears + tiny clock hands; Behavior: 1 main gear pierces, on hit sheds 2–3 hand splinters; Ult: Time-Snap Bubble (large slow AoE + repeated shard pings)

## 2.6 IDOLS (SYSTEM + ROSTER)

(Idols are permanent run modifiers equipped in 3 slots per hero. The game contains 12 total Idols. Idols are acquired and upgraded only through Boss Altars that appear after defeating midpoint and endpoint bosses (stages ending in 5 and 0).)

Idol Status Effects: Idols are the primary source of status effects applied to enemies (debuffs like slow, burn, poison, etc.). Each status effect has its own duration value that is defined per effect (exact duration numbers are tuned later).

Idol Slot Rule (locked): Each hero has 3 Idol slots from the start of the game. Idol slots do not unlock over time.

### 2.6.1 IDOL ALTAR (WORLD INTERACTABLE)

Boss Altar Spawn Rule: After defeating any midpoint boss (stage ending in 5) or endpoint boss (stage ending in 0) for a difficulty, a Boss Altar spawns immediately. The altar is used to acquire new Idols and upgrade existing ones. In co-op, each player makes their own independent pick/upgrade.

### 2.6.2 SELECTION LAYOUT (4×4 ALTAR + DRAG-TO-CLAIM)

Selection layout: a 4×4 altar field. The 12 Idols occupy the outer presentation ring around a central claim pad. The player drags an Idol into the center to lock it in.

### 2.6.3 IDOL ROSTER (12 TOTAL)

Idols have no rarity tiers. The Idol border color is based on element/theme. Each Idol has 3 upgrade tiers (T1/T2/T3).

- Example: Glue Idol — Create slow zones that evolve into corrosive traps.  T1: Hits drop Glue Puddles (slow).  T2: Puddles splash slow to nearby.  T3: Glue turns Corrosive (armor melt + DoT).

### 2.6.4 IDOL UPGRADES (STARS) - ALTAR ONLY

Idols upgrade from 1★ → 2★ → 3★. Upgrading is done only at the Boss Altar: dragging an Idol you already own into the central claim pad upgrades it (if an upgrade is available).

Rules (locked):

• No Idol Upgrade items exist (none are sold, dropped, or inventoried).

• Idol upgrades do not grant Power directly; they improve the Idol’s effects only.

• Idol upgrades do not affect Skulls or Difficulty (Difficulty Totems are the only difficulty source).

• Acquisition + Upgrades: Boss Altars appear after stage-ending-in-5 and stage-ending-in-0 bosses; each player claims/upgrades independently.

• If the chosen Idol is already 3★, the altar blocks upgrading it further and the player must choose a different Idol.

## 2.7 ITEMS (SYSTEM + FIELDS)

Items are the primary run-build modifiers that shape the player’s Auto Attack and survivability throughout a run. Every non-cursed, non-idol-upgrade item also grants Power (an additive % increase to Damage + Attack Speed + Scale).

Item Rule: Only Auto Attack and White Items are true damage sources. All other items are modifiers that change how Auto Attack behaves (AOE, scaling, DOT, etc.).

Power Rule (locked): PowerGivenPercent is a single additive % value (example: +5%) that increases Damage + Attack Speed + Scale equally. Power stacks additively across items.

Item Categories (Core):

- Normal Items (Black / Red / Yellow): Grant Power + unique EffectLines. Can be purchased and sold. Do NOT affect difficulty.

- White Items: Grant the MOST Power and also trigger White Mode Events (true damage). Obtained from Loot Bags. White items cannot duplicate (uniqueness ON).

- Cursed Items (Grey-Black): Loot Bag only. Cannot be purchased. Sell-only. Give no Power and only apply curse effect lines. Can be discarded. Do NOT affect difficulty.

- (No Idol Upgrade Items): Idol upgrades are not items. Idols are acquired and upgraded only at Boss Altars.

Design Rule (locked): There are no separate “essential stat-only” items. Damage / Attack Speed / Scale are always granted through Power on items.

Economy Fields (tuned later):

- BuyValue: Gold cost when purchased from the Vendor (Normal items only). White and Cursed items cannot be purchased.

- SellValue: Gold returned when sold (Normal items + Cursed items).

Data Schema (required fields; values tuned later):

- ItemID / DisplayName

- ItemRarity (loot bag rarity tier; includes White)

- ItemColorCategory (Black / Red / Yellow / White / CursedGreyBlack)

- BuyValue (Normal items only)

- SellValue (Normal items + Cursed items only)

- PowerGivenPercent (single additive % that increases Damage + Attack Speed + Scale equally; Normal items + White items only)

- EffectLines (1–3 text lines shown under Power in the tooltip; Cursed items use curse-only lines)

- Difficulty Source Note: Items do NOT carry any difficulty/skull increase fields. Difficulty comes only from Difficulty Totems.

- AcquisitionSources (VendorPurchase, LootBagDrop)

- ProcProfileID / ProcParameters (only for items that use the Proc system; values tuned later)

- Probability/Weight Fields (where random selection applies):

- ItemWeightWithinRarity (loot pool weighting)

- Any system-specific selection weight if the item can be rolled from multiple pools

Tooltip Display Rules (locked):

- Top-left: SellValue (if the item can be sold).

- Top line: PowerGivenPercent.

- Under Power: EffectLines.

Player Experience System (core concept): The run is tuned around two parallel experience pillars—Combat Experience System and Gambling Experience System. Both are primarily shaped by items (PowerGivenPercent + EffectLines) and are central to how the game should feel moment-to-moment.

Combat Experience System (internal tuning): By knowing expected item drop counts + rarity odds, the game can compute an expected Power total at different points in the run. This expected Power curve is used to ensure the player feels intentionally overpowered at designed moments while still allowing interesting item choices.

Gambling Experience System (internal tuning): A parallel tuning axis to Combat Experience. This tracks how often the player engages with RNG systems (slots, wheels, cash trucks, loot rolls) and how those outcomes trend over time for anti-cheat validation (abnormal streaks or counts can be flagged).

Anti-Cheat Provenance Rule (locked): Every owned item must map to a valid acquisition event record (Vendor purchase OR Loot Bag drop).

## 2.8 PROC SYSTEM (LOCKED)

(Defines what can trigger proc effects, how often, and how proc stacking works. Proc behavior is locked here so it is never decided in Unreal later.)

Core Proc Rules:

• Proc triggers are OnHit only. OnKill is not a proc trigger in Tribulation 66.

• Only Auto Attacks generate OnHit proc checks. Ultimates, abilities, environmental damage, and other sources never generate proc rolls.

• OnHit occurs once per Auto Attack activation (one attack instance), not per projectile and not per enemy hit.

• A proc roll happens at the first valid enemy contact during that Auto Attack activation. If the activation never hits an enemy, no proc roll occurs.

• Multiple non-White procs can trigger off the same OnHit event (multi-proc is allowed).

• Each proc-capable source (item / idol effect / unique-enemy effect) defines its own internal cooldown in data.

• Proc damage is proc-sterile: damage caused by a proc does not generate additional OnHit events and cannot chain into more procs.

• Co-op ownership: procs are owned by the player whose Auto Attack activation created the OnHit event. There is no shared team proc pool.

White Item Proc Rules (Special Case):

• White items can only trigger one White proc per OnHit event.

• If multiple White items succeed their proc roll on the same hit, randomly choose one of the successful White items (equal chance) and trigger only that one.

• There is no global White-proc lockout timer. White items can roll again on the next hit normally, limited only by their own internal cooldowns.

## 2.9 IN-RUN STAGE FLOW

Enemy Death Feedback:

- Normal kills: enemies melt into the ground.

- Overkill kills: if damage exceeds remaining HP by a threshold, enemies launch flying off the map (Smash Bros style).

- White items guarantee overkill: first hit deals ~150% of mob max HP.

Enemy Catch-Up Teleport (Anti-Ditch Rule): If the player gets too far from active wave enemies, they burrow down and reappear near the player to keep waves engaged.

Wave Budgets + Enemy Point Value:

- Wave composition is driven by a point budget.

- Every enemy has a PointValue used to build waves and support stage scoring caps.

Enemy Data Schema (Required Fields): MovementProfile, BaseDamage, AttackPatternID/Descriptor, PointValue.

(How each stage plays from spawn to exit: timer pressure, waves, miasma, and boss encounters.)

Each stage in Tribulation 66 is short, deterministic, and aggressively time-pressured. The stage timer does not start until the player crosses the entry gate into the main square map. When the gate is crossed, the countdown begins at 6:66. Miasma starts rising from the first second of the timer and continues rising throughout the stage, collapsing the playable space until full coverage at 0. When the timer reaches 0 and the map is fully covered, miasma damage becomes unavoidable and will tick the players down until death.

Enemy waves arrive in rapid bursts. The tuning target is 6–9 waves per stage, roughly one minute per wave. The next wave does not begin until the last wave-counting enemy dies and any active Unique enemies have been resolved (killed or exploded), which creates a clear rhythm: survive the burst, clear the field, then face the next burst.

Co-op Revive: In co-op, a dead teammate can be revived by interacting once (tap Interact). This triggers a short defibrillator revive animation that grants brief i-frames to the reviver. The revived player returns with exactly one heart and gains a short Supercharged buff (faster movement + faster attacks) for a few seconds.

Co-op Economy: Gold is personal per player (not shared).

Each normal stage contains four color-coded corner houses that act as interactable safe bubbles (Vendor, Gambler, Saint, Ouroboros). Players can shop, gamble, detour, or avoid danger while time keeps running. Exception: Boss-Only Checkpoint Stages do not include these houses and are a clean boss arena only.

Wave composition is driven by a point budget. The wave director spends the budget on wave-counting enemies. Unique enemies can spawn at any time as pressure spikes and do not replace the core wave population. Mini-bosses may appear with roughly a 50% chance per wave, capped at 1 per wave. All non-boss enemies are melee-only.

Unique Enemies (Pressure Spikes + Hero Status Source):

• Unique enemies can appear during waves as pressure spikes, and Stage Bosses can also summon Unique enemies during boss fights.

• Unique enemies are temporary. If not killed, they explode automatically after a short duration.

• Unique explosions deal damage and apply the Unique’s hero-facing status effect.

• Being hit by a Unique explosion counts as the Unique hitting the hero (it triggers the status + the heart visual effect change).

• Unique enemies are valid targets for Manual Attack Lock, so the player can click-lock them and focus them down.

• If the player kills a Unique enemy before its timer ends, the explosion does not happen (preventing the damage/status).

• Unique enemies do not drop loot bags or items.

• Wave-clear requirement: Unique enemies must be resolved by being killed or by exploding before the wave is considered cleared.

Skulls (Burden) affect future spawns only. When a wave begins, the wave director snapshots the current Burden tier and uses it for that wave’s scaling. Already-spawned enemies do not change retroactively when Burden changes mid-wave.

Normal Stage Boss Encounter: In normal stages, the Boss Quarters arena is reached after the wave phase; entering the arena automatically starts the boss fight (boss becomes active; it was already spawned at map generation).

Boss-Only Checkpoint Stages (Locked): Stages ending in 5 or 0 are checkpoint boss stages. The map is Start Area + Boss Quarters only, and the boss is already present at map generation (visible from the Start Area). These stages spawn no normal waves and contain no corner houses.

Cowardice Gate (Skip Option): In normal stages, the Trickster spawns at the entrance to the boss approach path and brings the Cowardice Gate with him. In boss-only checkpoint stages, the Cowardice Gate is pre-placed at map generation at the boundary between the Start Area and the Boss Quarters. Using the gate skips forward and marks the skipped boss as owed.

Skipped Boss Arena Rounds (Boss Catch-Up): Before entering a boss arena for an owed boss, the Boss Quarters is treated as a clean encounter space (no enemies carry over). On entry, the owed boss fight begins immediately.

At key checkpoints (Stage 10, 20, 30, 40, 50, and 60), the stage boss is an Epic Boss drawn from the Lesser Key of Solomon. The Epic Boss lineup is fixed by checkpoint: Stage 10 = Bael; Stage 20 = King Paimon; Stage 30 = Asmodeus; Stage 40 = Astaroth; Stage 50 = Belphegor; Stage 60 = Belial.

After a boss is killed, the immediate combat threat ends and a stage-exit gate appears that leads to the next stage. Players leave through the stage-exit gate to proceed to the next stage or difficulty checkpoint. Players can stay indefinitely after the boss is dead (timer is gone and miasma has receded) and leave through the gate whenever they choose.

Co-op Stage Exit: If any one player enters the stage-exit gate, it force-pulls all players into the next stage.

Downed Transition Rule: If a player is downed/dead when the force-pull happens, they are pulled forward but arrive in the next stage still downed (they must be revived by teammates).

Heat + Flow State (Ultra Instinct) (Locked): Heat is built by taking damage and by successfully dodging attacks (confirmed by attacks that would have hit but were avoided during dodge i-frames; touch damage counts). Heat can build slowly during normal wave combat and regular stage bosses, but Flow State can only trigger during checkpoint boss fights. Trigger: If Heat is full and the player takes a hit that would remove their last heart during a checkpoint boss fight, Flow State triggers instead of death. Flow State rules: the player is set to 0 hearts and receives 30 seconds to defeat the boss. If the boss dies before the timer ends, the player survives and regains 1 heart; if time expires, the player dies. Flow State can trigger at most once per boss fight, and at most twice per difficulty segment total (one use reserved for the stage-5 checkpoint boss and one use reserved for the stage-10 checkpoint boss). During Flow State, the boss switches to a cowardice pattern: it actively runs away while attacking to create distance. Flow State swaps to a unique portrait and music override for the hero.

## 2.10 WORLD INTERACTABLES (SYSTEM)

Enemy Death Feedback: When an enemy dies normally, it melts into the ground. When an enemy is overkilled (massive excess damage), it is launched flying like a Super Smash Bros knockback instead of melting.

(Interactive stage objects that appear during combat. Gameplay-first rules below; visuals are secondary.)

### 2.10.1 RARITIES + SPAWN RULES

Spawn RNG Fields (Luck Input): Each world interactable type declares SpawnChance/Weight, MinCountPerStage, MaxCountPerStage.

Covered types: Tree of Life, Wheel Spins, Cash Trucks (and Cash Truck Mimics), Boss Altars (Idols), Difficulty Totems, plus any future interactables.

World Interactables are generated each stage and placed using distance rules (no stacking on the player; some spawn farther than others). They spawn as one of 4 rarities: Black (lowest), Red, Yellow, White (highest). Distribution is left-weighted (Black/Red common; Yellow rare; White extremely rare). Rarity changes reward ranges and (where relevant) difficulty. They are placed during procedural generation at stage load and do not spawn mid-wave. In co-op, each player may interact with a world interactable once before it becomes consumed.

### 2.10.2 TREE OF LIFE

On interact: increases Max Hearts (adds new heart slots) and clears all current status effects. Max Hearts gained is a random roll based on rarity thresholds/ranges (MinHearts→MaxHearts per rarity). Tree of Life cannot be sold and does not exist as an inventory item—its effects apply immediately.

### 2.10.3 WHEEL SPIN (FORTUNE WHEEL)

Spawn RNG Fields (Luck Input): WheelSpawnChance/Weight, MinWheelsPerStage, MaxWheelsPerStage.

Outcome RNG Fields (Luck Input): SliceWeightTable (Black/Red/Yellow/White) + payout ranges per slice. White slice = jackpot.

Note: Wheel Spin counts and White-slice frequency feed Luck Rating; abnormal counts can trigger review.

On interact: opens a Wheel Popup overlay while the stage continues (same family of movable/minimizable popups as the Loot Pickup panel). The overlay shows a visible spinning prize wheel; the player always receives a reward. Most slices are Black rarity, fewer are Red, fewer are Yellow, and one is White (jackpot). Outcome awards rewards scaled by slice rarity; Luck influences slice weighting and payout magnitude.

### 2.10.4 CASH TRUCK / MIMIC

Spawn RNG Fields (Luck Input): TruckSpawnChance/Weight, MinTrucksPerStage, MaxTrucksPerStage.

Mimic RNG Field (Luck Input): MimicChance (chance a Cash Truck is a mimic).

Note: Mimic frequency contributes to Luck Rating (frequent mimics can represent “unlucky” outcomes).

Cash Truck: on interact, awards run Gold directly to the player wallet (no item drop). In co-op, each player can interact with a given truck once before it is consumed for them.

### 2.10.5 BOSS ALTAR (IDOLS)

Boss Altars grant Idols and upgrades (see IDOLS section). A Boss Altar spawns immediately after defeating midpoint and endpoint bosses (stages ending in 5 and 0). Each player makes an independent pick/upgrade at the altar while stage time continues.

### 2.10.6 DIFFICULTY TOTEMS (SKULLS)

Spawn RNG Fields (Luck Input): TotemSpawnChance/Weight, MinTotemsPerStage, MaxTotemsPerStage, and TotemRarityWeightTable (Black/Red/Yellow/White).

On interact: the totem activates instantly and increases Skulls for the entire party immediately. Totems are the ONLY source of difficulty increase in the game.

Skull Values (locked): Black +0.5 skull, Red +1 skull, Yellow +3 skull, White +5 skull. Half-skulls are displayed.

Visual Stack Rule (locked): The first activated totem plays a sound but does not create a stack. Each subsequent activation raises the previously activated totems out of the ground, lifting the newly activated totem upward so the stack becomes taller and taller over the run.

Anti-Cheat Integration: Totem spawn counts, rarity outcomes, and activation sequence are logged as Luck inputs. Abnormal totem rarity patterns or impossible counts are treated as cheating signals.

## 2.11 GLOBAL SPECIAL ENEMIES (SYSTEM)

Spawn RNG Fields (Luck Input): Each special enemy type declares SpawnChance/Weight, MinPerStage, MaxPerStage.

Additional Field: LuckImpactValue (can be negative). Example: Goblin Thief spawning is “unlucky” and can reduce Luck contribution.

- Cash Truck Mimic: Spawns from Cash Trucks that were generated as Mimics; it attempts to run the player over. If it hits, the player takes damage and is launched 'Super Smash' fly-off style. Killing it grants gold and may drop bonus loot (rarity-scaled).
- Goblin Thief: Steals value and attempts to escape; killing it returns value plus a bonus (rarity-scaled).
- Leprechaun: Flees and tries to waste your time; killing it grants bonus gold/loot roll value (rarity-scaled).

### 2.11.1 Specials

Global specials can appear in any stage and do not count toward wave-clear conditions. Waves end only when all wave-counting enemies are dead and any active Unique enemies have been resolved (killed or exploded). Global specials are rarity-scaled in strength.

### 2.11.2 Core Rules

(Three rare enemies that can spawn randomly in every stage, separate from the normal wave roster.)

## 2.12 PAUSE MENU

(Opens when pausing during a run.)

### 2.12.1 Visual Description

The Pause Menu appears as a centered overlay panel on top of paused gameplay. The gameplay view is visible behind it, darkened and frozen. The menu contains this exact vertical list of options: Resume Game, Save and Quit Game, Restart, Settings.

Resume Game closes the Pause Menu instantly and returns to gameplay. Save and Quit Game takes the player to the Main Menu (tooltip: “Takes you to the Main Menu”). In co-op, Save and Quit Game is Host-only. Restart immediately restarts the run with the current setup. In co-op, only the Host can press Restart and it does not require permission from other players. Settings opens the same Settings modal used from the Main Menu.

Restart Behavior: Restart immediately resets the run back to the Tutorial Entry start position. Restart can be triggered from this menu or via the Restart Run keybind. In co-op, only the Host can trigger Restart. Restarts do not play the meteor-drop entry animation.

Co-op Pause Authority: If any player pauses, the game pauses for everyone. Only the player who paused can unpause. On-screen overlay: “Paused by <SteamName>” and “Only <SteamName> can unpause.”

Offline Run Status (Top-Left): If the run is not eligible for leaderboards (started offline, went offline mid-run, or submissions were toggled off), the Pause Menu shows a small status line in the top-left reading: OFFLINE RUN — DOES NOT COUNT FOR LEADERBOARDS. This is informational only and does not prevent the player from finishing the run.

Co-op Disconnect Rule: If any player disconnects, the run autosaves immediately and the entire co-op session ends for all players. Show a blocking modal: PLAYER DISCONNECTED — “Someone disconnected. To continue this run, reload the game with everyone.” Button: Go to Load Game. The run becomes ineligible for leaderboards, but Achievements still unlock normally.

## 2.13 VENDOR (BLACK HOUSE)

(The Vendor is a shop + theft + debt system that creates risk/reward decisions mid-run.)

### 2.13.1 SYSTEM RULES

- Not a pause: stage time continues while the Vendor UI is open.
- Safe bubble: All NPCs (Vendor, Gambler, Ouroboros, Saint, Trickster) are surrounded by an always-active bubble that enemies cannot enter. Enemies also cannot spawn inside the bubble or close enough to clip into it. Miasma is blocked while the player is inside the bubble. Progression/cowardice gates are never placed near safe bubbles. If the Loan Shark is active, it circles the outside of the bubble menacingly.
- Inventory: sells Normal Items only. Cursed items are NEVER sold by the Vendor. (Idols are acquired/upgraded at Boss Altars only.)
Idol Upgrades: The Vendor never provides Idol upgrades. Boss Altars are the only way to acquire or upgrade Idols.

- Dialogue: Interacting with the Vendor opens a short dialogue. Teleporting to other houses is disabled once the boss encounter has begun (once Boss Quarters are entered).
- Loans: borrow gold any time; repay later at any Vendor/Gambler interaction. Leaving a stage with unpaid debt triggers the Loan Shark system.
- Stealing: attempt to steal a displayed item via a timing bar. Timing influences success odds.
- Anger: increases on failed steals. If anger reaches the stage threshold, the Vendor becomes an extra boss for that stage (in addition to the normal stage boss). Anger resets at the end of every stage. When the threshold is hit, the Vendor ends the interaction with a short “That’s enough!” animation and closes the UI.
Boss Defeat Reset: If the Vendor becomes an extra boss and is defeated, the Vendor returns to normal immediately. The Black House remains interactable for the rest of the stage, and the Vendor’s Anger Bar resets to empty.

Repeat Boss Scaling (per difficulty): The Vendor can be made angry again later in the same difficulty if the player tries to steal/cheat again. If the Vendor becomes a boss again in the same difficulty after already being beaten once, the Vendor returns at 2× strength. In practice, the Vendor should not be beatable more than once per difficulty due to extreme scaling.

Unique Reward (first defeat per difficulty): The first time the Vendor becomes an extra boss and is defeated in a given difficulty, the player receives a special reward and a permanent run modifier for that difficulty (Cash Trucks can no longer be Mimics for the rest of that difficulty).

### 2.13.2 LOANS

Borrow any amount. Repay cost = Principal + Fee. Default tuning target: +10% fee (values tunable).

### 2.13.3 ANGER + STEALING RULES

Steal Model (Matrix-Driven; tunable later): AttemptAmount -> (AngerThresholdIncrease, BaseSuccessDifficulty).

Derived Field: SuccessProbabilityFunction (computed from BaseSuccessDifficulty + future modifiers).

Note: Steal success/failure outcomes feed Luck Rating.

- Anger threshold target: 1200g of anger (tunable).
- Failed steal anger gain (value-weighted): Timing miss = +ItemPrice × 1.0; Timing hit but RNG fail = +ItemPrice × 0.5.
- Steal attempt uses a timing/QTE bar plus baseline odds (value/rarity scales difficulty).

### 2.13.4 VENDOR UI (VISUAL)

(Opens when interacting with the Vendor during a run. Stage time continues while open. Vendor bubble blocks enemy contact and blocks Miasma.)

The Vendor UI is a full-screen shop interface. The world remains active in the background, but an always-active safe bubble exists around the Vendor: enemies cannot enter and Miasma is held back while the player is inside it. The stage timer continues ticking. Players cannot attack/shoot while inside the bubble (attack input is ignored).

The shop layout shows a clear list of items for sale, each presented as a large horizontal tile with an icon, name, and short description. Prices are displayed with a gold icon, and the player’s current gold is visible.

Selecting an item tile purchases it instantly and the tile updates to show it is sold out or replaced depending on Vendor rules. A Back button closes the Vendor UI and returns to normal gameplay.

## 2.14 GAMBLER (YELLOW HOUSE)

(The Gambler is a risk/reward wagering system with optional cheating and escalating consequences.)

### 2.14.1 SYSTEM RULES

Gambling Outcome RNG Fields (Luck Input): GambleOutcomeProbabilityTable (win tiers / loss tiers / jackpot tiers).

Note: Gambling win frequency contributes to Luck Rating.

- Not a pause: stage time continues while the Gambler UI is open.
- Safe bubble: All NPCs (Vendor, Gambler, Ouroboros, Saint, Trickster) are surrounded by an always-active bubble that enemies cannot enter. Enemies also cannot spawn inside the bubble or close enough to clip into it. Miasma is blocked while the player is inside the bubble. Progression/cowardice gates are never placed near safe bubbles. If the Loan Shark is active, it circles the outside of the bubble menacingly.
- Games (v1.0): Rock Paper Scissors, Find the Ball, Coin Flip.
- Dialogue: Interacting with the Gambler opens a short dialogue with two options: “Let me gamble” (opens the casino UI) and “Teleport me to your brother” (teleports to the Vendor). Teleporting is disabled once the boss encounter has begun (once Boss Quarters are entered).
- Loans: borrow gold any time; repay later. Leaving a stage with unpaid debt triggers the Loan Shark system.
- Cheating: optional timing/QTE attempt to improve odds. Cheating increases anger even when successful.
- Anger: increases based on cheat value. If anger reaches the stage threshold, the Gambler becomes an extra boss for that stage (in addition to the normal stage boss). Anger resets at the end of every stage. When the threshold is hit, the Gambler ends the interaction with a short “That’s enough!” animation and closes the UI.
Boss Defeat Reset: If the Gambler becomes an extra boss and is defeated, the Gambler returns to normal immediately. The Red House remains interactable for the rest of the stage, and the Gambler’s Anger Bar resets to empty.

Repeat Boss Scaling (per difficulty): The Gambler can be made angry again later in the same difficulty if the player cheats again. If the Gambler becomes a boss again in the same difficulty after already being beaten once, the Gambler returns at 2× strength. In practice, the Gambler should not be beatable more than once per difficulty due to extreme scaling.

Unique Reward (first defeat per difficulty): The first time the Gambler boss is defeated in a given difficulty, it grants a massive amount of Score + Gold and drops a White Bag containing a unique White item: Guaranteed Jackpots (all Wheel Spins land on the White slice (jackpot) for the rest of that difficulty).

### 2.14.2 ANGER + CHEATING RULES

Cheat Model (Matrix-Driven; tunable later): AttemptAmount -> (AngerThresholdIncrease, BaseSuccessDifficulty).

Derived Field: SuccessProbabilityFunction (computed from BaseSuccessDifficulty + future modifiers).

Note: Cheat success/failure outcomes feed Luck Rating.

Cheating anger is value-weighted. Define ValueCheated = the gold value of the cheated advantage for that play:

- On cheat success: Anger += ValueCheated × 0.25
- On cheat failure: Anger += ValueCheated × 1.0
- Stage anger threshold rolls per stage (tuning target: 700–1400g of anger; values tunable).

### 2.14.3 CHEAT SUCCESS FEEDBACK (LOCKED)

On a successful cheat: play a short success SFX and show a small “Perfect”-style badge pop-up (combo-game vibe), then play the corresponding cheat animation:

- Rock Paper Scissors: you subtly change your hand symbol at the last moment.
- Find the Ball: you put on heat-seeking goggles that reveal the result.
- Coin Flip: you pull out a magnet to force the outcome.

### 2.14.4 GAMBLER UI (VISUAL)

(Opens when interacting with the Gambler during a run. Stage time continues while open. Gambler bubble blocks enemy contact and blocks Miasma.)

The Gambler UI is a full-screen gambling interface. The world remains active in the background, but an always-active safe bubble exists around the Gambler: enemies cannot enter and Miasma is held back while the player is inside it. The stage timer continues ticking. Players cannot attack/shoot while inside the bubble (attack input is ignored).

The layout presents gamble options as large tiles or buttons, each with a cost and a potential reward outcome. The UI emphasizes risk vs reward, with bold cost numbers and a satisfying “roll” presentation.

A Back button closes the Gambler UI and returns to gameplay.

## 2.15 LOAN SHARK (DEBT COLLECTOR)

(Triggered when a player leaves a stage with unpaid borrowed gold.)

### 2.15.1 SYSTEM RULES

If a player leaves a stage with unpaid debt, a Loan Shark spawns and aggressively chases that specific player. Debt is personal per player in co-op. Loan Sharks cannot enter NPC safe bubbles; instead, they circle the bubble perimeter menacingly while waiting.

Partial repayment is allowed. Paying back some of the debt makes the Loan Shark slower and weaker, but it only despawns once the full debt is repaid at the next Vendor or Gambler interaction. The Loan Shark is killable but extremely strong. Killing the Loan Shark does NOT forgive the debt: if the debt is still unpaid, another Loan Shark spawns on the next stage at 2× strength. On kill, the Loan Shark grants a large amount of Score + Gold and drops a White Bag containing a unique White item: Land Shark (you become a shark swimming through the ground with your fin visible; getting close to normal enemies eats them, while bosses take damage instead).

Proximity Warning OST (Locked): As the Loan Shark gets closer to the player, its OST fades in and ramps dynamically in intensity/volume based on distance. Loan Shark music overrides all other music layers while active so it reads as an immediate threat.

Death Feedback (Locked): If the Loan Shark kills the player, the death launches the player flying off the map with a Super Smash-style knockback send-off.

### 2.15.2 DEBT SCALING (LOCKED SLOT; VALUES TUNABLE)

Borrowing directly increases Loan Shark power. The system maps Debt → Tier, and each tier increases Loan Shark Health + Damage multipliers.

- Tiering concept: Tier 1 = small debt … Tier 5 = massive debt (breakpoints TO BE DECIDED).
- Each tier increases Loan Shark chase pressure in addition to raw stats (exact multipliers TO BE DECIDED).

## 2.16 RED HOUSE - OUROBOROS

Danger Signaling (Locked): The Red House visually broadcasts danger via menacing aura waves within a radius, warning the player as they approach.

(A dangerous in-run landmark that acts as a living environmental threat.)

Spawn Restriction (Locked): Ouroboros does not appear until Medium difficulty and onward (Stages 11+). Ouroboros occupies its house zone as a persistent, high-threat landmark. The area around Ouroboros is ringed with “DO NOT ENTER” danger signs and skull warnings.

Safe bubble rule: Ouroboros is surrounded by the same no-enemy bubble system as the other NPCs (enemies cannot enter). The bubble is visually marked as a dangerous perimeter rather than a comforting shelter.

Ouroboros is killable, but is intentionally extremely difficult. Ouroboros can only be killed once per run.

On kill: grants a massive amount of Score + Gold and drops a White Bag containing a unique White item: Reversal of Time (only procs when you are about to die; it triggers once and reverses time to prevent that death).

After Ouroboros is killed, the Yellow House remains on the map but stops being a danger zone for the rest of the stage (the threat is disabled).

## 2.17 WHITE HOUSE - SAINT

(A dangerous decision-point landmark hosted by the Saint.)

### 2.17.1 SYSTEM RULES

Dialogue: Interacting with the Saint opens a dialogue. Saint line: “You have no kromer.” The only response is “...”. The player can press Esc / Back to cancel the dialogue at any time.

Saint Beam Aura: If enemies enter the Saint’s aura, the Saint kills them with a continuous beam of light. If the player is caught in the beam path, the player is killed as well. Enemies killed by the Saint count toward wave-clear conditions, but grant no score and cannot drop loot bags or items.

Kromer Ending: When the player beats the final boss at Stage 66, they obtain exactly one kromer. The Saint appears and can finally accept the kromer. Giving the kromer completes the run and beats the game.

The White House is a calm, high-contrast landmark in the stage that contains the Saint. In normal stages, the Saint does not provide run-altering offerings; the interaction is purely flavor (“You have no kromer.” → “...”). The Saint’s primary gameplay function is the Saint Beam Aura and the Stage 66 kromer ending.

## 2.18 TRICKSTER (COWARDICE GATE ANNOUNCER)

### 2.18.1 SYSTEM RULES

The Trickster is an in-run NPC that appears only on normal stages (stages NOT ending in 5 or 0). He spawns at the entrance to the narrow approach path leading into the Boss Quarters, arriving with a short animation.

Cowardice Gate Delivery: When the Trickster spawns, he brings the Cowardice Gate and places it at the entrance to the boss approach path. The gate provides the option to skip the current boss (marking it as owed) or proceed into the fight.

Announcer Cutscene: When the party proceeds into the Boss Quarters, the Trickster performs a short announcer-style intro cutscene and then the match begins immediately.

Safe bubble: The Trickster is surrounded by the standard NPC safe bubble (enemies cannot enter). If the Loan Shark is active, it circles the outside of the bubble while waiting.

# 3. ITEM DROPS (SYSTEM + ENEMY LOOT BAGS)

(Items are acquired primarily through Loot Bags dropped from enemies. Each enemy kill rolls for a Loot Bag drop. A Loot Bag rolls a rarity tier using a probability table, then rolls a single Item from that rarity's loot pool table.)

## 3.1 LOOT BAG DROP (ENEMY DEATH ROLL)

Required Fields (Luck Input): LootBagDropChance, MinBagsPerStage, MaxBagsPerStage.

Note: Loot bag drop outcomes and bag counts contribute to Luck Rating (unusually high counts push Luck upward).

Enemies have a per-archetype Loot Bag drop chance that is rolled once on death.

- Each enemy archetype has LootBagDropChancePercent (defined in the Enemy Loot Drop Table).
- On enemy death, roll once. Success spawns a Loot Bag at the death location. Failure spawns nothing.
- Loot Bag drops are per-enemy, not per-wave.
Stage Seed Range: each stage has a seeded expected range for how many Loot Bags can drop overall, enforcing guaranteed minimums and preventing extreme dry streaks.

- Mini-bosses may use a different Loot Bag drop chance (or a guaranteed drop) as defined in the table. Unique enemies do not drop Loot Bags or items.

## 3.2 LOOT BAG RARITY ODDS (PROBABILITY TABLE)

Required Field (Luck Input): RarityWeightTable (includes White rarity; tunable later).

Note: White bag occurrence is a high-impact Luck event.

When a Loot Bag spawns, it immediately rolls its rarity tier.

- Rarity tiers are defined by the game (e.g., Common / Uncommon / Rare / Epic / Legendary / White).
- Rarity odds are defined in the Loot Bag Rarity Odds Table (a final probability table).
- Loot bags may use a LootBagProfileID when different contexts require different odds profiles (enemy-based or stage-based).

## 3.3 ITEM ROLL WITHIN RARITY (LOOT POOL TABLES)

Required Fields (Luck Input): ItemWeightWithinRarity, MinItemsPerStage, MaxItemsPerStage.

Note: Item acquisition outcomes contribute to Luck Rating and support anti-cheat sanity checks.

After rarity is selected, the bag rolls 1 Item from the loot pool for that rarity tier.

- Each rarity tier has a dedicated Loot Pool Table that defines exactly which Items are eligible.
- The bag rolls exactly 1 Item from the chosen rarity pool (unless explicitly overridden by a special rule).
- Items may be equal-weight or weighted; weights are stored inside the Loot Pool Table.
- Loot bags can contain Normal Items, White Items, and Cursed Items. Cursed items are Loot Bag only (never sold by the Vendor).

## 3.4 OPTIONAL LOCK-IN RULES (ONLY IF USED)

(These rules are now locked for v1.0 unless explicitly changed later.)

- Duplicate protection: OFF. Non-White items can duplicate and stack; repeats are allowed and desirable.
- White item uniqueness: ON. White items cannot duplicate; once obtained, that specific White item is removed from the White loot table.
White back-to-back lockout: ON. Two White rarity rolls in a row is a Forbidden Event (Cheating Certainty).

- Pity / anti-dry-streak uplift: ON. Loot Bags enforce guaranteed minimums so the player always gets meaningful drops.
- Stage-based rarity bias: OFF.
- Enemy-based rarity bias: Mini-bosses + Stage Bosses only (exact bias rules DECIDE LATER).
- Item unlock gating: OFF (no discovery gating).
Co-op ownership (LOCKED): Loot Bags are instanced per player. Each player can receive their own bag from combat drops, and only the owning player can collect it.

# 4. FRONTEND - AFTER THE RUN

## 4.1 RUN SUMMARY PAGE

Damage Breakdown Display Rules (locked):

- Do NOT show Auto Attack as its own row in the breakdown.

- Show White Items as true damage source entries.

- Show non-white items and idols as “Damage Contribution” entries (how much they added to Auto Attack damage).

Power Integration Rule (locked): Power is not shown as its own row. Its effect is included inside each item’s damage contribution math (because Power increases Auto Attack output).

Luck Rating (Run-End Metric): Luck Rating aggregates every RNG roll across the run and evaluates how lucky/unlucky the outcomes were.

Luck Rating uses bounded distributions: each stage has capped min/max outcome boundaries for key RNG events (bags, slots, cheats, etc.).

If Luck Rating > 100, the run is flagged as “Too Lucky” for review (also appears in Account Status Panel).

Dodge Rating (Run-End Metric): Dodge Rating aggregates confirmed dodge events across the run (including touch damage avoided during dodge i-frames) and evaluates both dodge quality and timing consistency. This rating is shown on the Run Summary Page alongside Luck Rating and is also used by the anti-cheat system to flag “Dodging too perfect” when the dodge profile is not human-plausible.

(Shown after every run. Also opens when clicking a Global Top-10 leaderboard row, when opening your own locally saved best run from Friends view, or when clicking Run In Question.)

The Run Summary Page is the standard post-run results screen shown after every run. It is also used for leaderboard integrity review: Global Top-10 entries expose a full run summary with build snapshot and event log, while Friends leaderboards do not host online run logs (only your own best run can be opened locally).

The page is divided into three clear vertical regions: Left, Center, and Right. This layout makes it possible to understand the run’s build, outcomes, and legitimacy at a glance, while still allowing deep inspection for leaderboard integrity.

Checkpoint Clear Rule: After beating a difficulty checkpoint boss and choosing Continue, the Bounty score and Speedrun timer reset for the next difficulty segment. The Run Summary also includes Save and Quit options that return to the Main Menu (tooltip: “Takes you to the Main Menu”).

Left Region (Run Identity + Rankings): The left side stacks the run’s identity in descending order: Hero portrait + name, Companion portrait + name, Stage reached, Bounty value (Score), and Time elapsed. Below that are the weekly global rankings for Bounty and Speedrun. If the run did not clear the required difficulty checkpoint, the Speedrun rank is blank while the Bounty rank may still be shown.

Left Region (Ratings + Logs): Below rankings are the Luck Rating and Dodge Rating, plus the detailed run log. These are exposed for deeper validation and scoring analysis when needed.

Center Region (Build Snapshot): The center region shows the player’s chosen Idols in a 2x3 grid at the top. Below it is the player’s final inventory shown in a 3x3 grid with paging. Items are sorted with White items first, and then by rarity tiers.

Right Region (Damage + Reporting): The right region contains a damage breakdown designed to highlight which parts of the build contributed most to damage output, including item contribution tracking (exact math defined elsewhere). If the run enters the leaderboard, two additional elements appear: an owner-only Video Link box where the run owner can paste a YouTube link for the run (with Copy and Open actions), and a This Run is Cheating button visible to viewers for Global Top-10 leaderboard runs. Reporting opens a modal where players submit a short reason and optional evidence link. Reporter identity is private, reports are rate-limited (per-minute and per-day caps), and submitting a report does not automatically remove or hide the run; it simply sends the report to developer review and moderation.

# 5. GAME OVERVIEW

## 5.1 FANTASY / TONE

## 5.2 OPEN SYSTEMS - TO BE DECIDED

Resolved in V15 Integration (locked rules; numbers tunable later):

- Luck Rating model: bounded probability inputs across the entire run; Luck > 100 flags “Too Lucky.”

- Anti-cheat model: hard caps + event consistency + provenance validation + Luck Rating (no generic suspicion score).

- Combat damage model: Auto Attack + White Items are true sources; all other items/idols are modifiers; damage breakdown rules locked.

- Enemy baseline schema fields: MovementProfile, BaseDamage, AttackPatternID/Descriptor, PointValue.

- Wave construction rule: point-budget spawns driven by Enemy PointValue (supports score caps).

- Economy schema fields: every item has BuyValue/SellValue; every gold source has Value.

- Vendor/Gambler steal/cheat: matrix-driven model (attempt amount -> anger increase + base success difficulty).

Still Open / To Be Decided (tunable math + content details):

- Luck Rating exact normalization/scoring curve (how each RNG event contributes numerically).

- Dodge Rating exact weighting/thresholds (inputs locked; tuning later).

- Final economy numbers (item prices, sell values, gold sources, payout tuning).

- Gambler payout caps and scaling (maximum wins, limits, progression).

- Exact wave spawn math (point budgets per stage, mini-boss chance curves, special pacing).

- Enemy attack pattern library (specific move sets per enemy; behaviors beyond baseline schema).

- Hero stat scaling + UI surfacing (which stats are displayed, growth rules, balance targets).

# 6. APPENDICES

## 6.1 STAGES (1–66)

Stages Appendix has been moved to a separate file to keep the main Bible lightweight: TRIBULATION 66 – BIBLE STAGES (v1.0).

## 6.2 HEROES

This appendix lists the hero roster. Each hero is defined only by Auto Attack behavior and one Ult (no hero passives, no rivals).

Alice in Wonderland Rabbit — Auto: Spinning pocket-watch gears + tiny clock hands; Behavior: 1 main gear pierces, on hit sheds 2–3 hand splinters; Ult: Time-Snap Bubble (large slow AoE + repeated shard pings)

Lu Bu — Auto: Red crescent halberd arc blades; Behavior: Wide arc pierces, every 3rd shot becomes a double-arc; Ult: Sky-Piercing Halberd (giant line projectile + detonation)

Leonardo da Vinci — Auto: Clockwork bees/drones; Behavior: Bounce once off walls, on hit break into 2 micro-bolts that seek; Ult: Renaissance Engine (deploy orbiting turret)

Yakub (Alchemist/Artificer) — Auto: Alchemical sigils launching rune-seed shards; Behavior: Marks target, next 1–2 shots home harder + tiny DoT; Ult: Grand Formula (multi-pulse transmutation circle + lingering zone)

King Arthur — Auto: Golden sword-lances; Behavior: Straight shot pierces 1, on kill triggers a micro holy splash; Ult: Excalibur Radiance (sustained beam sweep / line blast)

Miyamoto Musashi — Auto: Two alternating flying slashes (long/short); Behavior: Every 4th shot echoes an afterimage slash; Ult: Niten Cyclone (spinning AoE + outward slashes)

Captain Jack Sparrow — Auto: Spinning gold coins + occasional mini cannonball; Behavior: Coins ricochet 1–2 times, cannonball causes small knock AoE; Ult: Black Pearl Broadside (ghost ship volley rains cannon shots)

Solo Leveler (Ranker) — Auto: Shadow daggers; Behavior: Fast shots chain to 1 extra target (reduced chain damage); Ult: Shadow Army (summons soldiers that add projectile streams)

Saitama — Auto: Air-punch shockwaves; Behavior: Short cone that pierces, every N punches releases a ring burst; Ult: Serious Punch (lane-deleting shockwave)

RoboGoon — Auto: Tri-burst rounds; Behavior: Center pierces, side shots stagger; every N bursts fires an armor-break debuff round; Ult: Auto-Target Protocol (perfect-aim barrage)

George Washington — Auto: Heavy musket balls; Behavior: Slow strong shot pierces 1, on hit creates 2–3 fragment fan; Ult: Continental Volley (layered lane fire)

Cain — Auto: Jagged shards with dark cracks; Behavior: Stacking bleed, at max stacks target pops in small AoE; Ult: Mark of Cain (marked zone takes amplified damage + falling shards)

Billy the Kid — Auto: Silver bullets; Behavior: Rapid shots ricochet once, every N fires a penetrator line; Ult: Deadeye (manual aim cone/line of guaranteed crits)

Roach King (Asmon-inspired) — Auto: Grime shards + roach bits; Behavior: Hits apply tiny DoT, every N hits spawns a micro-roach that bites nearby; Ult: Roach Swarm (flood of roaches multi-biting packs)

Goblino (XQC-inspired) — Auto: Green impact orbs; Behavior: Bouncy shots with tiny knock; Ult: Streamer Rampage (Winston-like berserk: leap + slam AoE chain)

BulkBite (CaseOh-inspired) — Auto: Snack-chunk projectiles; Behavior: Chunky hits splash crumbs; Ult: Snack-Man (rush mode: move fast and "chomp" enemies in your path)

HoboWanderer (Forsen-inspired) — Auto: Rusty can tosses + broken bottle shards; Behavior: short arc lobs that bounce/ricochet once; Ult: Alley Stampede (a rolling shopping cart wave that plows a lane, then bursts into shrapnel).

DryHumor (MoistCr1TiKaL-inspired) — Auto: Deadpan rapid-fire pings; Behavior: Low-effort lane clearing with constant stagger pressure; Ult: Unbothered Rampage (locks aim steady + damage spikes while ignoring flinch)

LyricVoi (LIRIK-inspired) — Auto: Echo shots; Behavior: Projectiles leave afterimage hits; Ult: Clipstorm (rapid highlight barrage)

Jesterma (Jerma-inspired) — Auto: Prank pellets; Behavior: Shots randomly fake-out with sudden knock; Ult: Bit Collapse (screen-warp burst zone)

NorthKing (Northernlion-inspired) — Auto: Regal coins; Behavior: Ricochet logic favors walls; Ult: Commentary Flood (coin rain + confusion)

Peakwarden (Summit-inspired) — Auto: Mountain shards; Behavior: Long-range lane pressure; Ult: High Ground (creates a ridge line that buffs shots)

QuinnHex (Quinn69-inspired) — Auto: Hex bolts; Behavior: Stacks a curse, pops on cap; Ult: Build-Then-Boom (huge delayed detonation)

SandSultan (Esfand-inspired) — Auto: Sand blasts; Behavior: Wide coverage cones; Ult: Oasis Guard (protective zone that blasts enemies)

CharNut (Burnt Peanut-inspired) — Auto: Ember nuts; Behavior: Splinters into sparks; Ult: Roast Wave (flame sweep)

Wraithveil (Shroud-inspired) — Auto: Clean tracer lines; Behavior: Precision lane shots with high impact; Ult: Phantom Scope (scope overlay + manual critical shots)

## 6.3 ITEMS

This appendix lists all non-White item Series and their tier scaling (Basic / Great / Hyper).

## APPENDIX - NON-WHITE ITEM SERIES

This appendix lists all non-White item Series. Each Series shares one icon (sprite) across its tiers; tier is represented only by border color. Multiple copies can be collected and stack together. The three tiers are Basic (Black border), Great (Red border), and Hyper (Yellow border).

Damage Increase: Basic (Black Border): Slightly increase damage. Great (Red Border): Moderately increase damage. Hyper (Yellow Border): Greatly increase damage.

Attack Speed Increase: Basic (Black Border): Slightly increase attack speed. Great (Red Border): Moderately increase attack speed. Hyper (Yellow Border): Greatly increase attack speed.

Attack Scale Increase: Basic (Black Border): Slightly increase scale. Great (Red Border): Moderately increase scale. Hyper (Yellow Border): Greatly increase scale.

Extra Cash Truck Spawns: Basic (Black Border): Chance for an extra Cash Truck to spawn as a higher tier.; Small chance for an extra Cash Truck to be white.

Extra Wheel Spin Spawns: Basic (Black Border): Chance for an extra Wheel Spin to spawn per stage. Better tiers increase the chance and/or number of extra Wheel Spins. Small chance for an extra Wheel Spin to spawn as a Jackpot Wheel (guaranteed White slice).

Jackpot Chance + Payout: Basic (Black Border): Chance to trigger a Jackpot. Great (Red Border): Higher chance to trigger a Jackpot.; Moderately increase Jackpot payout. Hyper (Yellow Border): High chance to trigger a Jackpot.; Greatly increase Jackpot payout.; Chance a Jackpot upgrades into a MegaJackpot.

Extra Tree Spawns: Basic (Black Border): Chance for an extra Tree to spawn. Great (Red Border): Higher chance for an extra Tree to spawn.; Chance a spawned Tree rolls a higher tier. Hyper (Yellow Border): High chance for an extra Tree to spawn.; High chance a spawned Tree rolls a higher tier.; Small chance for an extra Tree to be white.

Extra Leprechaun Spawns: Basic (Black Border): Chance for an extra Leprechaun to spawn. Great (Red Border): Higher chance for an extra Leprechaun to spawn.; Chance a spawned Leprechaun rolls a higher tier. Hyper (Yellow Border): High chance for an extra Leprechaun to spawn.; High chance a spawned Leprechaun rolls a higher tier.; Small chance for an extra Leprechaun to be white.

Extra Goblin Thief Spawns: Basic (Black Border): Chance for an extra Goblin Thief to spawn. Great (Red Border): Higher chance for an extra Goblin Thief to spawn.; Chance a spawned Goblin Thief rolls a higher tier. Hyper (Yellow Border): High chance for an extra Goblin Thief to spawn.; High chance a spawned Goblin Thief rolls a higher tier.; Small chance for an extra Goblin Thief to be white.

Cash Truck Mimic Detection + Bonus Mimic Loot: Basic (Black Border): Enable Mimic detection ping/indicator.; Additional Mimic bonus loot roll(s) (flat +N).; Chance a Mimic drops a higher-tier bag.

Vendor Price Reduction: Basic (Black Border): Slightly reduce Vendor prices. Great (Red Border): Moderately reduce Vendor prices.; Small chance a Vendor item is Free. Hyper (Yellow Border): Greatly reduce Vendor prices.; Chance a Vendor item is Free.; Small chance a Vendor item is Free.

Gamble Win Chance: Basic (Black Border): Chance to win a gamble. Great (Red Border): Higher chance to win a gamble.; Moderately increase gamble payout. Hyper (Yellow Border): High chance to win a gamble.; Greatly increase gamble payout.; Chance to win a gamble.

Steal Success Chance: Basic (Black Border): Chance to successfully steal when stealing is attempted. Great (Red Border): Higher chance to successfully steal when stealing is attempted. Hyper (Yellow Border): High chance to successfully steal when stealing is attempted.

Cheat Success Chance: Basic (Black Border): Chance to successfully cheat when cheating is attempted. Great (Red Border): Higher chance to successfully cheat when cheating is attempted. Hyper (Yellow Border): High chance to successfully cheat when cheating is attempted.

Dash Cooldown Reduction: Basic (Black Border): Slightly reduce dash cooldown. Great (Red Border): Moderately reduce dash cooldown. Hyper (Yellow Border): Greatly reduce dash cooldown.

Stage-Effect Strength Increase: Basic (Black Border): Slightly increase stage-effect strength. Great (Red Border): Moderately increase stage-effect strength.; Small chance stage effects double-tick (apply twice). Hyper (Yellow Border): Greatly increase stage-effect strength.; Chance stage effects double-tick (apply twice).; Small chance stage effects roll White intensity.

Stage-Effect Ignore Chance: Basic (Black Border): Chance stage effects deal damage to you. Great (Red Border): Moderately increase (MISC_CHANCE).; Small chance stage effects also apply no debuff to you. Hyper (Yellow Border): Greatly increase (MISC_CHANCE).; Chance stage effects also apply no debuff to you.; Small chance you gain brief stage-effect immunity after a successful ignore.

Rubble Decay Speed Increase: Basic (Black Border): Slightly increase rubble decay speed. Great (Red Border): Moderately increase rubble decay speed. Hyper (Yellow Border): Greatly increase rubble decay speed.

Loot Pickup Radius Increase: Basic (Black Border): Slightly increase loot pickup radius. Great (Red Border): Moderately increase loot pickup radius. Hyper (Yellow Border): Greatly increase loot pickup radius.

Evade Chance Increase: Basic (Black Border): Chance to evade an incoming hit. Great (Red Border): Higher chance to evade an incoming hit.; Small chance to counterattack after an evade. Hyper (Yellow Border): High chance to evade an incoming hit.; Chance to counterattack after an evade.; Small chance for the counterattack to execute.

Damage Taken Reduction: Basic (Black Border): Slightly reduce damage taken. Great (Red Border): Moderately reduce damage taken.; Moderately increase reflected damage. Hyper (Yellow Border): Greatly reduce damage taken.; Greatly increase reflected damage.

Heart on Kill Chance: Basic (Black Border): Chance a kill gives you a heart. Great (Red Border): Higher chance a kill gives you a heart.; Small chance the heart is doubled. Hyper (Yellow Border): High chance a kill gives you a heart.; Chance the heart is doubled, Small chance its hearts.

Elemental Damage Reduction: Basic (Black Border): Slightly reduce elemental damage taken. Great (Red Border): Moderately reduce elemental damage taken. Hyper (Yellow Border): Greatly reduce elemental damage.; Chance to ignore an elemental damage instance.

Daze Chance on Hit: Basic (Black Border): Chance to Daze on hit. Great (Red Border): Higher chance to Daze on hit.; Small chance to Sleep on hit. Hyper (Yellow Border): High chance to Daze on hit.; Chance to Sleep on hit.; Small chance slept enemies wake Confused (brief ACC loss).

Enemy Aggro Range Reduction: Basic (Black Border): Slightly reduce enemy aggro range. Great (Red Border): Moderately reduce enemy aggro range. Hyper (Yellow Border): Greatly reduce enemy aggro range.; Chance you are not noticed when entering an area.

Critical Hit Chance + Crit Tier Upgrades: Basic (Black Border): Chance to critically hit. Great (Red Border): Higher chance to critically hit.; Small chance a crit upgrades into a Mega Crit (stronger crit tier). Hyper (Yellow Border): High chance to critically hit.; Chance a crit upgrades into a Mega Crit (stronger crit tier).; Small chance a crit upgrades into an Ultra Crit (even stronger crit tier).

Close-Range Damage Increase: Basic (Black Border): Slightly increase close-range damage. Great (Red Border): Moderately increase close-range damage.; Small chance to critically hit at close range. Hyper (Yellow Border): Greatly increase close-range damage.; Chance to critically hit at close range.; Small chance to execute at close range.

Bounce-Jump + Stomp Damage: Basic (Black Border): Enable bounce-jump (jump on enemies for a boost). Great (Red Border): Moderately increase stomp damage.; Small chance for a stomp to critically hit. Hyper (Yellow Border): Greatly increase stomp damage.; Chance for a stomp to critically hit.; Small chance for a stomp to execute.

Taunt Chance + Enemy Miss Chance: Basic (Black Border): Chance to Taunt on hit. Great (Red Border): Higher chance to Taunt on hit.; Moderately increase chance of enemy missing attack. Hyper (Yellow Border): High chance to Taunt on hit.; Greatly increase chance of enemy missing attack.; Chance to apply Berserk on taunt.

Execute Chance (Risky): Basic (Black Border): Chance to execute.; Chance to be executed (downside). Great (Red Border): Higher chance to execute.; Higher chance to be executed (downside). Hyper (Yellow Border): High chance to execute.; High chance to be executed (downside).

Thorns Damage Return: Basic (Black Border): Slightly increase returned damage (Thorns). Great (Red Border): Moderately increase returned damage (Thorns).; Small chance for Thorns to crit. Hyper (Yellow Border): Greatly increase returned damage (Thorns).; Chance for Thorns to crit.; Small chance for Thorns to execute.

Heart Fruit Drop Chance: Basic (Black Border): Chance an enemy drops a Heart Fruit on kill. Great (Red Border): Higher chance an enemy drops a Heart Fruit on kill.; Chance a dropped Heart Fruit is higher rarity. Hyper (Yellow Border): High chance an enemy drops a Heart Fruit on kill.; High chance a dropped Heart Fruit is higher rarity.; Small chance a dropped Heart Fruit is White.

Summon Ally Spawn Chance: Basic (Black Border): Chance to spawn a summon ally. Great (Red Border): Higher chance to spawn a summon ally.; Small chance a summon spawns as a power-up instead. Hyper (Yellow Border): High chance to spawn a summon ally.; Chance a summon spawns as a power-up instead.; Chance a summon triggers Beast Mode.

Trap Spawn Chance: Basic (Black Border): Chance to spawn a trap. Great (Red Border): Higher chance to spawn a trap.; Chance a trap spawns as a Big Trap. Hyper (Yellow Border): High chance to spawn a trap.; High chance a trap spawns as a Big Trap.; Small chance a trap spawns as a Mega Trap (Executes).

Mirror Image Spawn Chance: Basic (Black Border): Chance to spawn a Mirror Image. Great (Red Border): Higher chance to spawn a Mirror Image.; Small chance the Mirror Image deals damage. Hyper (Yellow Border): High chance to spawn a Mirror Image.; Greatly increase Mirror Image damage.; Small chance the Mirror Image can execute.

Size Increase: Basic (Black Border): Slightly increase character size. Great (Red Border): Moderately increase character size, Increase attack scale. Hyper (Yellow Border): Greatly increase character size, Increase attack scale, Increase Damage.

Size Reduction: Basic (Black Border): Slightly reduce character size. Great (Red Border): Moderately reduce character size, Increase attack speed. Hyper (Yellow Border): Greatly reduce character size, Increase attack speed, Increase Damage.

Stage Boss Damage Increase: Basic (Black Border): Slightly increase damage vs Stage Bosses. Great (Red Border): Moderately increase damage vs Stage Bosses.; Small chance to crit vs Stage Bosses. Hyper (Yellow Border): Greatly increase damage vs Stage Bosses.; Chance to crit vs Stage Bosses.; Small chance to execute vs Stage Bosses.

Espada Damage Increase: Basic (Black Border): Slightly increase damage vs Espadas. Great (Red Border): Moderately increase damage vs Espadas.; Small chance to crit vs Espadas. Hyper (Yellow Border): Greatly increase damage vs Espadas.; Chance to crit vs Espadas.; Small chance to hyper crit Espadas.

Boss Damage Increase: Basic (Black Border): Slightly increase damage vs Stage Bosses. Great (Red Border): Moderately increase damage vs Stage Bosses; small chance to crit Stage Bosses. Hyper (Yellow Border): Greatly increase damage vs Stage Bosses; chance to crit Stage Bosses; small chance to hyper-crit Stage Bosses.

Siblings Damage Increase: Basic (Black Border): Slightly increase damage vs Siblings (Vendor/Gambler). Great (Red Border): Moderately increased damage vs Siblings (Vendor/Gambler).; Small chance to crit vs Siblings (Vendor/Gambler). Hyper (Yellow Border): Greatly increase damage vs Siblings (Vendor/Gambler).; Chance to crit vs Siblings (Vendor/Gambler).; Small chance to hyper crit vs Siblings (Vendor/Gambler).

Biped Damage Increase: Basic (Black Border): Slightly increase damage vs Bipeds. Great (Red Border): Moderately increase damage vs Bipeds.; Small chance to crit vs Bipeds. Hyper (Yellow Border): Greatly increase damage vs Bipeds.; Chance to crit vs Bipeds.; Small chance to execute vs Bipeds.

Quadruped Damage Increase: Basic (Black Border): Slightly increase damage vs Quadrupeds. Great (Red Border): Moderately increase damage vs Quadrupeds.; Small chance to crit vs Quadrupeds. Hyper (Yellow Border): Greatly increase damage vs Quadrupeds.; Chance to crit vs Quadrupeds.; Small chance to execute vs Quadrupeds.

Flying Damage Increase: Basic (Black Border): Slightly increase damage vs Flying enemies. Great (Red Border): Moderately increase damage vs Flying enemies.; Small chance to crit vs Flying enemies. Hyper (Yellow Border): Greatly increase damage vs Flying enemies.; Chance to crit vs Flying enemies.; Small chance to execute vs Flying enemies.

Unique Enemy Damage Increase: Basic (Black Border): Slightly increase damage vs Unique enemies. Great (Red Border): Moderately increase damage vs Unique enemies.; Small chance to crit vs Unique enemies. Hyper (Yellow Border): Greatly increase damage vs Unique enemies.; Chance to crit vs Unique enemies.; Small chance to execute vs Unique enemies.

Mini-Boss Damage Increase: Basic (Black Border): Slightly increase damage vs Mini-Bosses. Great (Red Border): Moderately increase damage vs Mini-Bosses.; Small chance to crit vs Mini-Bosses. Hyper (Yellow Border): Greatly increase damage vs Mini-Bosses.; Chance to crit vs Mini-Bosses.; Small chance to execute vs Mini-Bosses.

Biped Spawn Weighting Increase: Basic (Black Border): Slightly increase Biped spawn weighting. Great (Red Border): Moderately increase Biped spawn weighting.; Moderately reduce non-Biped spawn weighting. Hyper (Yellow Border): Greatly increase Biped spawn weighting.; Greatly reduce non-Biped spawn weighting.; Small chance the next wave is Biped-only.

Quadruped Spawn Weighting Increase: Basic (Black Border): Slightly increase Quadruped spawn weighting. Great (Red Border): Moderately increase Quadruped spawn weighting.; Moderately reduce non-Quadruped spawn weighting. Hyper (Yellow Border): Greatly increase Quadruped spawn weighting.; Greatly reduce non-Quadruped spawn weighting.; Small chance the next wave is Quadruped-only.

Flying Spawn Weighting Increase: Basic (Black Border): Slightly increase Flying spawn weighting. Great (Red Border): Moderately increase Flying spawn weighting.; Moderately reduce non-Flying spawn weighting. Hyper (Yellow Border): Greatly increase Flying spawn weighting.; Greatly reduce non-Flying spawn weighting.; Small chance the next wave is Flying-only.

Companion Passive Strength Increase: Basic (Black Border): Slightly increase companion passive effect strength. Great (Red Border): Moderately increase companion passive effect strength. Hyper (Yellow Border): Greatly increase companion passive effect strength.

Ultimate Cooldown Reduction: Basic (Black Border): Slightly reduce Ultimate cooldown. Great (Red Border): Moderately reduce Ultimate cooldown. Hyper (Yellow Border): Greatly reduce Ultimate cooldown.; Chance your Ultimate does NOT go on cooldown after use.

Discovery Reveal Radius Increase: Basic (Black Border): Slightly increase discovery reveal radius. Great (Red Border): Moderately increase discovery reveal radius. Hyper (Yellow Border): Greatly increase discovery reveal radius.

# APPENDIX - WHITE ITEMS (WHITE MODE EVENTS)

White Items are rare run-altering pickups that trigger a temporary White Mode Event. During a White Mode Event, the player gains a powerful themed ability for a short burst, and all other item procs are disabled until the event ends. White Items are designed to instantly kill normal enemies, and to deal percentage-based damage to bosses.

Fruit Ninja: White Mode Event: Hold attack and slash arcs to cut enemies intersecting the arc.

Jetpack Joyride: White Mode Event: Hold to hover/strafe; jet flame damages enemies you pass through.

Mega Mushroom: White Mode Event: Giant form; contact kills; stomps create shockwave clears.

Star Power: White Mode Event: Invincible speed; kills by contact aura; ranged disabled.

Rodeo Stampede: White Mode Event: Lasso + mount; steer stampede trample.

Get Over Here Flail: White Mode Event: Hook yank; enemy becomes spinning flail you swing to mow crowds.

Kirby Inhale: White Mode Event: Vacuum enemies; swallow one; spit as piercing cannonball.

Shadow Clone Swarm: White Mode Event: Spawn clones that mirror attacks with offsets; blender zone.

Aegis Phalanx: White Mode Event: Moving shield wall; push behind it, impaling ahead.

Shield Ricochet: White Mode Event: Throw shield; bounces between enemies and returns; blocks frontal contact while out.

Spirit Bomb: White Mode Event: Charge growing orb; release for massive sweep kill.

TRON Lightcycle: White Mode Event: Become bike; leave light-wall trail; enemies touching trail die.

Angry Birds Launch: White Mode Event: You are the projectile; impact AoE; chaining launches ramps damage.

DDR Shock Steps: White Mode Event: Rhythm steps; each success emits shock ring; streak extends.

Blink Execute Chain: White Mode Event: Blink behind target and execute; chain within a short window.

Portal Sinkhole: White Mode Event: Place portals; enemies fall/removed; hole grows while held.

Unluck Lightning Marker: White Mode Event: Marker locks; last-second dodge triggers lightning nuke.

Damage Bank Retaliation: White Mode Event: HP can’t drop below 1 heart; bank damage; end releases huge blast.

Rolling Thunder: White Mode Event: Roll; collisions knockback/damage; pin vs walls.

Katamari Ball: White Mode Event: Roll up enemies to grow; end explosion AoE.

Pac Pellet Eater: White Mode Event: Chomp dash; enemies panic; chaining briefly increases speed.

Spin Dash Rocket: White Mode Event: Hold charge then release; limited steering; timing through clusters matters.

Tony Hawk Trick Line: White Mode Event: Auto skate; rails/ramps; landings slam; combos extend.

Frogger Hops: White Mode Event: Movement becomes discrete hops; each hop emits AoE; streak ramps.

Cardboard Box: White Mode Event: Hide while still; pop-out tackle nukes; re-hide.

ODM Gear Flyby: White Mode Event: Grapple and swing; high-speed flybys slice enemies.

Overdrive Notes: White Mode Event: Kills drop notes; complete phrase → massive pulse clear.

Rewind Safety: White Mode Event: On near-death: rewind 2 seconds + burst at origin.

Pokeball Swarm: White Mode Event: Capture eligible mobs temporarily; they fight for you; throw multiple.

Checkpoint Rush: White Mode Event: Race to a beacon; no damage until first arrives; winner nukes the zone.

Mario Kart Blue Shell: White Mode Event: Targets strongest enemy; repeated detonations; herd enemies into it.

Tetris Drop: White Mode Event: Drop blocks to crush lanes; completing a line triggers a clear wave.

Hotline Breach: White Mode Event: Fast breach-dash executes; chaining extends the execute window.

AC-130 Overwatch: White Mode Event: Open a laptop to an overhead targeting view; enemies are outlined in red hitboxes; fire a heavy cannon barrage to mow down crowds.

Arcade Zombie FPS: White Mode Event: The whole screen shifts into an old-school arcade zombie shooter style; you play in first-person and gun down waves until the mode ends.

## 6.4 IDOLS

This appendix lists all Idols and their Tier 1–3 upgrade behaviors.

Glue Idol — Create slow zones that evolve into corrosive traps.  T1: Hits drop Glue Puddles (slow).  T2: Puddles splash slow to nearby.  T3: Glue turns Corrosive (armor melt + DoT).

Bomb Idol — Turn single-target into controlled AoE demolition.  T1: Hits explode small AoE.  T2: Explosions drop delayed micro-bombs.  T3: Blast-kills trigger a secondary blast.

Lightning Idol — Damage jumps between targets.  T1: Hit chains 1 bounce.  T2: Chains fork + apply Shock.  T3: Shocked enemies emit Arc Pulses over time.

Frost Idol — Slow → freeze → shatter packs for burst.  T1: Hits apply Chill stacks.  T2: At max stacks, enemy freezes.  T3: Hitting frozen enemies Shatters in a cone/splash.

Poison Idol — Stack poison then spread it.  T1: Poison stacks (DoT).  T2: At threshold, poison splashes nearby.  T3: Death leaves Toxic Cloud zone.

Net Idol — Turn autos into CC tools.  T1: Occasional Net → Root.  T2: Rooted enemies become Marked (bonus auto damage).  T3: Root triggers Snare Burst (mini pull + damage).

Sunder Idol — Make tanky enemies melt.  T1: Sunder stacks (armor down).  T2: At max stacks, enemy becomes Exposed (bonus damage).  T3: Exposed hits release a Fracture Wave line AoE + spreads Sunder.

Solar Idol — Big ground AoE endgame.  T1: Every few hits drops a Sun Brand burn circle.  T2: Brands merge into larger burn zone + Weaken.  T3: Large Solar Field persists and flare-pulses.

Black Hole Idol — Group enemies then collapse them.  T1: Gravity stacks spawn mini pull.  T2: Pull becomes short vortex.  T3: Vortex collapses into Implosion burst (executes low HP).

Light Idol — Turn autos into beams.  T1: Every Nth attack fires a Light Beam line.  T2: Beam applies Illuminate (bonus damage taken).  T3: Illuminated enemies emit Prismatic Bursts on hit.

Mirror Idol — Double attacks in windows.  T1: Every Nth hit spawns Mirror Echo (repeats hit).  T2: Echo Window copies next X attacks once.  T3: Echoed hits fire Mirror Shards (radial bolts).

Quake Idol — Fracture the ground.  T1: Hits create a Crack Line AoE.  T2: Cracks apply brief Stagger.  T3: Cracks erupt Aftershock Spines along the line.

Fire Idol — Ignite lanes and force movement.  T1: Ignite stacks (DoT).  T2: Ignited enemies splash a small flame ring on hit.  T3: Ignited deaths leave a Burning Trail zone.

Wind Idol — Push, pull, and bunch enemies.  T1: Gust stacks cause knockback.  T2: Gust becomes a short pull-in.  T3: Pull-in ends with a Wind Burst AoE.

Blood Idol — Convert aggression into sustain.  T1: Small lifesteal on hit.  T2: Overheal becomes temporary shield.  T3: Shield expiration releases a Blood Nova AoE.

Thorn Idol — Punish enemies for closing in.  T1: Nearby enemies take thorn ticks.  T2: Thorns apply brief slow on tick.  T3: Taking damage triggers a Thorn Burst AoE.

Time Idol — Freeze moments, then cash out.  T1: Every Nth hit causes Micro-Stasis slow.  T2: Stasis marks enemies (bonus damage).  T3: Marked enemies take periodic Clock Burst ticks.

Swarm Idol — Summon seekers automatically.  T1: Hits spawn a Swarm Bug that bites.  T2: Bugs split on kill.  T3: Bugs detonate in small AoE on expiry.

Sound Idol — Shockwaves for crowd control.  T1: Hits emit a tiny shockwave cone.  T2: Shockwave applies Stagger.  T3: Every few shockwaves releases a Sonic Ring (360°).

Void Idol — Collapsing damage zones.  T1: Hits leave a tiny Void Scar zone.  T2: Scars link into short lines.  T3: Linked scars collapse into a Void Pop burst.

## 6.5 ACHIEVEMENTS

This appendix defines the Achievements tiers and their Achievement Coin rewards.

T66 - Achievements Appendix (Full List)

This file defines the full Achievements list by rarity tier. Achievements grant Achievement Coins (AC) used for cosmetics.

Economy reference: 1 skin = 250 AC. Rewards are fixed per tier: Black = 25 AC, Red = 100 AC, Yellow = 250 AC, White = 1000 AC.

UI note: Each achievement row is intended to display Description (left), Progress (middle), and Reward Coins (right).

Reward per achievement: 25 AC

| ID | Achievement | Requirement | Progress Display | Reward |
| --- | --- | --- | --- | --- |
| ACH_BLK_001 | First Blood | Kill 1 enemy. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_002 | Wave Clear | Clear your first wave. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_003 | Gate Crasher | Cross the stage gate and start the stage timer. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_004 | Summoning Stone | Summon a stage boss using the Summoning Stone. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_005 | Stage 1 Clear | Clear Stage 1 in any run. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_006 | Stage 5 Clear | Clear Stage 5 in any run. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_007 | Stage 10 Clear | Clear Stage 10 in any run. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_008 | First Idol | Obtain an Idol from an Idol Altar. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_009 | Triple Idol | Equip 3 Idols at the same time. | 3/3 (in one run) | 25 AC |
| ACH_BLK_010 | Slot #2 Unlocked | Unlock your 2nd Idol Slot (reach Stage 10 at least once). | 1/1 (lifetime) | 25 AC |
| ACH_BLK_011 | Tree Hugger | Use the Tree of Life once. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_012 | Spin To Win | Use a Slot Machine once. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_013 | Treasure Taster | Open a Treasure Chest. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_014 | Mimic Hunter | Defeat a Mimic. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_015 | Goblin Payday | Defeat a Goblin Thief. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_016 | Leprechaun Catch | Defeat a Leprechaun. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_017 | Vendor Customer | Buy any item from the Vendor (Black House). | 1/1 (lifetime) | 25 AC |
| ACH_BLK_018 | Casino Visitor | Play any gamble option at the Gambler (Red House). | 1/1 (lifetime) | 25 AC |
| ACH_BLK_019 | Borrower | Borrow gold (take a loan) from Vendor or Gambler. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_020 | Honest Payer | Repay any debt. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_021 | Loot Bag | Pick up an item from a Loot Bag. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_022 | White Spark | Trigger a White Mode Event. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_023 | Crowd Control | Kill 20 enemies in 10 seconds. | 20/20 (in one run) | 25 AC |
| ACH_BLK_024 | Heart Collector | Reach 10 Hearts in a single run. | 10/10 (in one run) | 25 AC |
| ACH_BLK_025 | Burden Rising | Reach 10 Skulls (Burden) in a single run. | 10/10 (in one run) | 25 AC |
| ACH_BLK_026 | Map Reader | Open the Full Map overlay at least once. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_027 | HUD Juggler | Toggle any HUD panel 10 times. | 10/10 (lifetime) | 25 AC |
| ACH_BLK_028 | Clip Watcher | Open the in-run Media Viewer panel. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_029 | Settings Tinkerer | Change any setting mid-run. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_030 | Co-op Savior | Revive a teammate in a co-op run. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_031 | After Action Report | View the Run Summary Page after a run ends. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_032 | Leaderboard Tourist | Change any leaderboard filter at least once. | 1/1 (lifetime) | 25 AC |
| ACH_BLK_033 | Save And Quit | Use Save and Quit during a run. | 1/1 (lifetime) | 25 AC |

Reward per achievement: 100 AC

| ID | Achievement | Requirement | Progress Display | Reward |
| --- | --- | --- | --- | --- |
| ACH_RED_001 | Clean Stage | Clear a stage without taking any damage. | 1/1 (in one stage) | 100 AC |
| ACH_RED_002 | Perfect Wave | Clear 10 waves without taking any damage. | 10/10 (lifetime) | 100 AC |
| ACH_RED_003 | Boss Rush | Summon and defeat a boss before the stage timer reaches 4:00. | 1/1 (in one stage) | 100 AC |
| ACH_RED_004 | Last-Second Escape | Defeat a boss with 10 seconds or less left on the stage timer. | 1/1 (in one stage) | 100 AC |
| ACH_RED_005 | Debt Dodger | Clear a stage while you still have unpaid debt. | 1/1 (in one stage) | 100 AC |
| ACH_RED_006 | Loan Shark Slayer | Defeat the Loan Shark. | 1/1 (lifetime) | 100 AC |
| ACH_RED_007 | Sticky Fingers | Successfully steal an item from the Vendor. | 1/1 (lifetime) | 100 AC |
| ACH_RED_008 | Cheat Day | Successfully cheat in a Gambler minigame. | 1/1 (lifetime) | 100 AC |
| ACH_RED_009 | Anger Management | Reach 80% anger with Vendor or Gambler and still leave safely. | 1/1 (lifetime) | 100 AC |
| ACH_RED_010 | Vendor Boss Down | Cause the Vendor to become a boss and defeat it. | 1/1 (lifetime) | 100 AC |
| ACH_RED_011 | Gambler Boss Down | Cause the Gambler to become a boss and defeat it. | 1/1 (lifetime) | 100 AC |
| ACH_RED_012 | Ouroboros Dare | Enter Ouroboros danger radius and survive for 30 seconds. | 30/30 (in one stage) | 100 AC |
| ACH_RED_013 | Saint's Judgment | Have the Saint beam-kill 10 enemies in a single stage. | 10/10 (in one stage) | 100 AC |
| ACH_RED_014 | No Score Needed | Clear a stage where the Saint kills at least 5 enemies. | 1/1 (in one stage) | 100 AC |
| ACH_RED_015 | Idol Collector | Obtain 10 different Idols across all runs. | 10/10 (lifetime) | 100 AC |
| ACH_RED_016 | Idol Master | Upgrade any one Idol to Tier 3. | 1/1 (lifetime) | 100 AC |
| ACH_RED_017 | Full Board | Equip 6 Idols at the same time. | 6/6 (in one run) | 100 AC |
| ACH_RED_018 | Wave Marathon | Clear 66 waves across all runs. | 66/66 (lifetime) | 100 AC |
| ACH_RED_019 | 66 In 66 | Kill 66 enemies within 66 seconds. | 66/66 (in one stage) | 100 AC |
| ACH_RED_020 | Crit Machine | Land 50 critical hits in one run. | 50/50 (in one run) | 100 AC |
| ACH_RED_021 | Execute Artist | Execute 25 enemies in one run. | 25/25 (in one run) | 100 AC |
| ACH_RED_022 | Status Dealer | Apply any status effect to 25 enemies in one run. | 25/25 (in one run) | 100 AC |
| ACH_RED_023 | Treasure Tycoon | Earn 5,000 gold in a single run. | 5000/5000 (in one run) | 100 AC |
| ACH_RED_024 | Jackpot Winner | Hit a Slot Machine jackpot at least once. | 1/1 (lifetime) | 100 AC |
| ACH_RED_025 | Chest Runner | Open 10 Treasure Chests in one run. | 10/10 (in one run) | 100 AC |
| ACH_RED_026 | Lucky Streak | Get 3 Loot Bags in a row (3 consecutive bag spawns). | 3/3 (in one run) | 100 AC |
| ACH_RED_027 | White Collector | Trigger 3 White Mode Events in one run. | 3/3 (in one run) | 100 AC |
| ACH_RED_028 | White Mode Chain | Trigger 3 different White Mode Events in one run. | 3/3 (in one run) | 100 AC |
| ACH_RED_029 | No Companion, No Problem | Clear Stage 20 with NO COMPANION equipped. | 1/1 (lifetime) | 100 AC |
| ACH_RED_030 | Trio Trouble | Clear a stage in a Trio co-op run. | 1/1 (lifetime) | 100 AC |
| ACH_RED_031 | Duo Deep | Clear 10 stages in Duo co-op across all runs. | 10/10 (lifetime) | 100 AC |
| ACH_RED_032 | Panel Lock Pro | Toggle-lock a HUD panel and then use the global HUD toggle. | 1/1 (lifetime) | 100 AC |
| ACH_RED_033 | Brotherly Visit | Use the Vendor-to-Gambler teleport (or reverse) in a stage. | 1/1 (lifetime) | 100 AC |

Reward per achievement: 250 AC

| ID | Achievement | Requirement | Progress Display | Reward |
| --- | --- | --- | --- | --- |
| ACH_YEL_001 | Ten Stages Untouched | Clear 10 stages in a row without taking damage. | 10/10 (in one run) | 250 AC |
| ACH_YEL_002 | One-Heart Champion | Kill a stage boss while you are on exactly 1 Heart. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_003 | Miasma Champion | Kill a boss with 15 seconds or less left on the stage timer. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_004 | 66-Second Stage | Clear a stage in under 66 seconds (gate to stage complete). | 1/1 (in one stage) | 250 AC |
| ACH_YEL_005 | 66-Second Boss | Summon and kill a boss within 66 seconds of summoning. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_006 | Full T3 Belt | Have all equipped Idols at Tier 3 at the same time. | 1/1 (in one run) | 250 AC |
| ACH_YEL_007 | Six Altars Visited | Interact with Idol Altars on stages 11/21/31/41/51/61 in a single run. | 6/6 (in one run) | 250 AC |
| ACH_YEL_008 | Jackpot Hat Trick | Trigger 3 jackpots in one run. | 3/3 (in one run) | 250 AC |
| ACH_YEL_009 | MegaJackpot | Trigger a MegaJackpot result. | 1/1 (lifetime) | 250 AC |
| ACH_YEL_010 | Perfect Heist Run | Successfully steal 3 items in one run without a failed steal. | 3/3 (in one run) | 250 AC |
| ACH_YEL_011 | Perfect Cheat Run | Successfully cheat 5 times in one run without a cheat failure. | 5/5 (in one run) | 250 AC |
| ACH_YEL_012 | Debt Titan | Carry 10,000+ debt into a stage and still clear that stage. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_013 | Loan Shark Speedbag | Kill the Loan Shark within 10 seconds of it spawning. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_014 | Ouroboros Dancer | Survive 66 seconds inside Ouroboros danger radius. | 66/66 (in one stage) | 250 AC |
| ACH_YEL_015 | Saint's Bait | Lure 25 enemies into Saint aura for kills in one stage (and survive). | 25/25 (in one stage) | 250 AC |
| ACH_YEL_016 | Team Guardian | Revive each teammate at least once in a single run (Duo/Trio). | 2/2 or 3/3 (in one run) | 250 AC |
| ACH_YEL_017 | Team Perfect Stage | Clear a stage in co-op with zero damage taken by the entire party. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_018 | No-Hit Boss Party | Kill a boss in co-op with zero damage taken by the entire party. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_019 | White Variety Pack | Trigger 3 different White Mode Events in a single run. | 3/3 (in one run) | 250 AC |
| ACH_YEL_020 | White Storm | Trigger 6 White Mode Events in a single run. | 6/6 (in one run) | 250 AC |
| ACH_YEL_021 | Overkill Arcade | Launch 66 enemies via overkill knockback across a single run. | 66/66 (in one run) | 250 AC |
| ACH_YEL_022 | 66 Crits | Land 66 critical hits in a single stage. | 66/66 (in one stage) | 250 AC |
| ACH_YEL_023 | 66 Executes | Execute 66 enemies in a single run. | 66/66 (in one run) | 250 AC |
| ACH_YEL_024 | Gold 66,666 | Hold 66,666 gold at one time. | 66666/66666 (in one run) | 250 AC |
| ACH_YEL_025 | Hearts 66 | Reach 66 Hearts in one run. | 66/66 (in one run) | 250 AC |
| ACH_YEL_026 | Burden 66 | Reach 66 Skulls (Burden) in one run. | 66/66 (in one run) | 250 AC |
| ACH_YEL_027 | Slot Assassin | Destroy 10 jackpot Slot Machines for their payouts in one run. | 10/10 (in one run) | 250 AC |
| ACH_YEL_028 | Special Sweep | Kill a Mimic, Goblin Thief, and Leprechaun in the same stage. | 3/3 (in one stage) | 250 AC |
| ACH_YEL_029 | Shop Speed | Buy 3 items from the Vendor in under 10 seconds. | 3/3 (in one visit) | 250 AC |
| ACH_YEL_030 | Casino Streak | Win 10 gambles in a row in one run. | 10/10 (in one run) | 250 AC |
| ACH_YEL_031 | Wave Cleaner | Clear all waves in a stage without being hit and without entering any House. | 1/1 (in one stage) | 250 AC |
| ACH_YEL_032 | Item Hoarder | Collect 30 items in one run. | 30/30 (in one run) | 250 AC |
| ACH_YEL_033 | House Tour | Interact with Black, Red, Yellow, and White Houses in one stage and still clear it. | 4/4 (in one stage) | 250 AC |

Reward per achievement: 1000 AC

| ID | Achievement | Requirement | Progress Display | Reward |
| --- | --- | --- | --- | --- |
| ACH_WHI_001 | Kromer Salvation | Beat Stage 66 and give the kromer to the Saint. | 1/1 (lifetime) | 1000 AC |
| ACH_WHI_002 | Betrayal Proof | Beat Stage 66 with NO COMPANION equipped. | 1/1 (lifetime) | 1000 AC |
| ACH_WHI_003 | Final Difficulty Conqueror | Beat Stage 66 on Final difficulty. | 1/1 (lifetime) | 1000 AC |
| ACH_WHI_004 | Unscathed To The End | Beat Stage 66 without taking damage at any point in the run. | 1/1 (in one run) | 1000 AC |
| ACH_WHI_005 | 66-Stage Marathon | Clear Stages 1-66 in one continuous run without Save and Quit. | 66/66 (in one run) | 1000 AC |
| ACH_WHI_006 | The Devil's Speedrun | Summon and kill the Stage 66 boss within 66 seconds. | 1/1 (in one stage) | 1000 AC |
| ACH_WHI_007 | White Lightning | Trigger 12 different White Mode Events in a single run. | 12/12 (in one run) | 1000 AC |
| ACH_WHI_008 | Jackpot Apocalypse | Hit 6 jackpots in one run and destroy every jackpot Slot Machine. | 6/6 (in one run) | 1000 AC |
| ACH_WHI_009 | House Dominance | In one run, defeat Vendor Boss, Gambler Boss, and Loan Shark. | 3/3 (in one run) | 1000 AC |
| ACH_WHI_010 | Ouroboros Master | Spend a total of 6 minutes inside Ouroboros danger radius across a run and still win. | 360/360 (in one run) | 1000 AC |
| ACH_WHI_011 | Saint's Mercy | Lure 66 enemies into the Saint aura across a run without being hit by the beam. | 66/66 (in one run) | 1000 AC |
| ACH_WHI_012 | Absolute 66 | Beat the final boss while holding 66 Hearts, 66 Burden, and 66,666 gold. | 1/1 (in one run) | 1000 AC |
