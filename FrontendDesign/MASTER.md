# Frontend Design Master

## Purpose

This folder is the source-of-truth reference for the T66 browser-first UI workflow.

The goal is:

1. replicate current Unreal UI screens in HTML/CSS/JS
2. lock the component hierarchy and screen structure
3. run controlled discovery to find the right style family
4. refine the approved direction to a professional bar
5. translate the approved look back into Unreal UMG/CommonUI

This process exists because browser iteration is faster, more visual, and easier to refine than trying to invent the final UI directly inside Unreal.

## Canonical Locations

- WebLab prototypes: `C:\UE\T66\SourceAssets\UI\WebLab`
- Main menu prototype: `C:\UE\T66\SourceAssets\UI\WebLab\screens\main-menu`
- WebLab exports/previews: `C:\UE\T66\SourceAssets\UI\WebLab\exports`
- Shared WebLab assets: `C:\UE\T66\SourceAssets\UI\WebLab\shared`
- Discovery skill: `C:\Users\DoPra\.codex\skills\game-ui-discovery`
- Production skill: `C:\Users\DoPra\.codex\skills\retro-indie-game-ui`
- Discovery skill file: `C:\Users\DoPra\.codex\skills\game-ui-discovery\SKILL.md`
- Production skill file: `C:\Users\DoPra\.codex\skills\retro-indie-game-ui\SKILL.md`
- Screen review gate: `C:\UE\T66\FrontendDesign\SCREEN_REVIEW_GATE.md`

## Split Workflow

There are now two distinct Codex skill modes for frontend screen work:

1. Discovery
   Use to explore multiple style families while keeping the component map fixed.
   Goal: find the right direction.
   Bar: exploration with professional mechanical hygiene.

2. Production
   Use after a direction is selected.
   Goal: refine the chosen direction to a professional, Unreal-ready screen.
   Bar: shippable-quality craft and polish.

## Core Workflow

### 1. Replicate The Unreal Screen

Start by matching the current in-game screen as closely as possible in HTML.

This replication phase is not for redesigning structure. It is for capturing:

- all screen regions
- all buttons and controls
- panel shells
- typography placement
- background composition
- animated layers
- the full component inventory

If something exists in Unreal, it should exist in the browser version before major redesign work starts.

### 2. Lock The Screen Skeleton

Once the screen is replicated, treat the structure as locked unless explicitly changed.

For each screen, preserve:

- top-level layout
- panel placement
- major action placement
- control group placement
- hierarchy names

Change the skin, not the skeleton.

### 3. Run Discovery Before Production

Discovery pass goals:

- compare shell languages
- compare title treatments
- compare button families
- compare typography families
- compare material stacks
- compare environment moods

Discovery pass restrictions:

- keep the layout and component map stable
- do not silently reuse previous generated screens as inspiration
- do not borrow backgrounds, titles, or surfaces from prior exploration without explicit approval
- do not present broken variants just because the style direction is interesting

### 4. Refine The Chosen Direction

Once the structure is locked and a direction is chosen, refine:

- background environment
- material language
- frame and trim design
- button plates
- typography
- iconography
- logo treatment
- ambient animation
- overall mood and visual identity

The approved browser version becomes the visual specification for the Unreal rebuild.

### 5. Translate To Unreal

The HTML is not the final runtime asset. It is the design lab and visual reference.

When rebuilding in Unreal, preserve:

- spacing
- anchors
- component hierarchy
- panel sizes
- state behavior
- timing and motion intent
- textures and materials

Avoid relying on browser-only effects that cannot be recreated cleanly in Unreal.

## Why HTML First

HTML/CSS/JS is currently the best environment for:

- fast visual iteration
- seeing full-screen composition immediately
- testing different materials and backgrounds
- evaluating typography and icon treatment
- trying multiple directions without heavy engine overhead

Unreal remains the final implementation target, but the browser prototype is where the look should be solved first.

## Active Skills

Discovery:

- `$game-ui-discovery`
- `C:\Users\DoPra\.codex\skills\game-ui-discovery`

Production:

- `$retro-indie-game-ui`
- `C:\Users\DoPra\.codex\skills\retro-indie-game-ui`

## Recommended Skill Usage

Example prompts:

- `Use $game-ui-discovery to explore 5 clearly different main-menu families while keeping the component map fixed and tracking asset provenance.`
- `Use $retro-indie-game-ui to take family 3 and polish it to a professional screen.`
- `Use $retro-indie-game-ui to score this selected screen for professional polish and list the top blockers.`

## Design Rules For T66

These rules are specific to the current project process.

### 1. Preserve Structure Early

At the beginning of a screen, do not redesign layout casually.

The early goal is:

- correct screen structure
- correct component inventory
- correct hierarchy naming

### 2. Prefer Authored Assets

Use real assets whenever possible:

- backgrounds
- altar or focal art
- trims
- fills
- fonts
- icons
- logo treatments

Do not try to fake richness entirely with basic CSS shapes.

### 3. Track Provenance

During discovery, every important visual ingredient must have a known source:

- canonical current screen
- explicit repo asset
- newly generated asset for this pass
- user-approved carryover

Do not treat "already exists in the repo" as permission to use a past generated variant as a hidden reference.

### 4. Avoid Generic Web UI Tells

Avoid:

- sterile flat rectangles
- decorative cut corners with no real material logic
- default dashboard spacing
- modern SaaS typography habits
- empty black voids behind floating widgets
- fake complexity that still reads as simple CSS

### 5. Make The Screen Feel Like A Place

Even a menu should feel like it exists inside a world.

The UI should not read as boxes over wallpaper. It should read as:

- shrine
- chamber
- control deck
- ritual table
- archive
- bunker
- workshop

depending on the requested vibe.

### 6. Do Not Call It Done Too Early

Improved is not done.

Every major pass should be judged by two separate scores:

- professional polish
- requested vibe fit

If the screen still looks like a prototype, say so directly and identify the real blockers.

### 7. Mechanical Polish Is Mandatory

Even discovery passes must clear basic craft checks.

Reject any pass with:

- text or components cut off
- accidental empty black margins around buttons
- off-center content that reads as a mistake
- title treatment using mismatched or accidental background content
- panel fills leaking outside their intended frames
- obvious control mis-sizing

## Screen Review Standard

For each major screen pass, aim to provide:

1. current status
2. file paths
3. preview image path
4. component hierarchy
5. top blockers
6. provenance note for non-canonical assets during discovery

All screen passes should also clear the workspace review gate in:

- `C:\UE\T66\FrontendDesign\SCREEN_REVIEW_GATE.md`

## Component Hierarchy Standard

Each screen should be described in stable layers or regions so future edits can target exact elements.

Example format:

1. top bar
2. top bar > left utility cluster
3. top bar > center navigation cluster
4. top bar > right utility cluster
5. left social panel
6. center action shell
7. right leaderboard shell
8. background stack
9. background stack > focal altar
10. background stack > atmosphere layers

Use this naming discipline consistently so later prompts can refer to exact elements.

## Current Direction Notes

The current main menu work is exploring a direction closer to:

- retro-inspired indie game UI
- cozy spooky apocalypse
- darker shrine or chamber mood
- stronger material richness
- less generic CSS geometry
- more authored atmosphere

The current bar is not "looks better than before."

The real bar is:

- could stand next to a polished published indie game
- feels intentional and memorable
- survives translation back into Unreal

## Operational Notes

- Keep browser prototypes in `SourceAssets/UI/WebLab`
- Keep exports in `SourceAssets/UI/WebLab/exports`
- Keep shared generated assets in `SourceAssets/UI/WebLab/shared`
- Keep screen-specific files grouped by screen
- Keep the hierarchy stable across redesign passes

## Future Expansion

This folder can later hold:

- screen briefs
- approved style directions
- UI token sheets
- font decisions
- logo evolution notes
- Unreal translation checklists

For now, `MASTER.md` is the single anchor document for the process.
