# Master Style

This document records the current design direction, process, and memory for the main menu visual identity pass.

This is a preliminary exercise intended to get close to the target look with the current ChatGPT image workflow. A later pass should be expected once newer OpenAI image models are available. The point of this file is to preserve direction, prompt logic, evaluation criteria, and reasons why individual attempts were close or not close.

## Memory

- This is the canonical style memory file for the frontend visual-identity work.
- This pass is preliminary and should be reusable as context for future OpenAI model passes.
- Current priority order: display font exploration first, altar exploration second, panels and borders later.
- For broad exploration batches where duplicate-looking generations are a risk, prefer fresh ChatGPT tab or fresh chat per request.
- For the display-font exploration phase, use `GO!` as the evaluation word instead of `NEW GAME`.
- For review convenience, `GO!` exploration sheets should contain `4` numbered variants per image.
- For altar exploration, generate `altar only` concepts first, not full composed scenes.

## Scope

This pass is intentionally limited to:

- the custom main menu display font direction
- the main menu background image direction

This pass does not yet cover:

- panel shells
- borders
- button frames
- tabs
- dropdown chrome
- full UI token replacement

The menu layout is not the target of this pass. The structure can remain mostly as-is while the visual identity changes.

## Core Intent

The target is not generic "retro UI."

The target is:

- indie retro
- PS1-adjacent rendering sensibility
- darker and more violent
- slightly gory
- cozy in the sense of being self-contained and visually disciplined
- not clean SaaS-like
- not glossy live-service
- not Fortnite-like

The closest high-level direction is:

- infernal shrine
- ritual monument
- low-fi display treatment
- contained, readable interface

The visual identity should feel like a dark ritual object rendered through a degraded old display pipeline, not a modern UI with retro filters pasted on top.

## Canonical Workflow

Use the existing Chrome-based ChatGPT art workflow:

- [ChatGPT_Image_Production_Workflow.md](/C:/UE/T66/Docs/Art/ChatGPT_Image_Production_Workflow.md)

Important rules inherited from that workflow:

- use one ChatGPT conversation per batch unless there is a reason not to
- preview one image first
- approve the look before running more
- do not resend prompts just because rate limiting happened
- harvest existing results before regenerating
- save the final settled render, not a transient one

## Existing Main Menu Asset Structure

The current main menu background is already split into three animated/source layers:

- `SourceAssets/UI/MainMenu/Animated/sky_bg.png`
- `SourceAssets/UI/MainMenu/Animated/fire_moon.png`
- `SourceAssets/UI/MainMenu/Animated/pyramid_chad.png`

Those are imported by:

- [ImportUIMainMenuAnimatedBackgroundTextures.py](/C:/UE/T66/Scripts/ImportUIMainMenuAnimatedBackgroundTextures.py)

And loaded by the main menu screen against the cooked `/Game/UI/MainMenu/*` textures:

- [T66MainMenuScreen.cpp](/C:/UE/T66/Source/T66/UI/Screens/T66MainMenuScreen.cpp)

That means the safest visual-direction rule is:

- preserve the three-layer structure
- preserve the current movement/composition idea
- replace the art inside the layers
- do not redesign the whole screen just to fix the art direction

## Decision: Custom Font

The current repo font pool is not the target direction for the frontend identity pass.

The current decision is to author a custom display font direction instead of trying to settle for an existing font family.

However, this must be framed correctly:

- this pass is for a custom display font
- this pass is not yet for a universal body/UI family

Reason:

- the title and major CTA text need a very specific tone
- small UI text, player names, settings labels, filters, and localized strings have much stricter readability constraints
- trying to solve both with one generated font is likely to fail or slow the project down

### Practical Constraint

`23 letters` is not enough.

Minimum glyph set for the display-font pass:

- `A-Z`
- `0-9`
- basic punctuation: `. , : ; ! ? - ' " / & +`

If the font will appear in buttons or screen headers, it also needs:

- space behavior that looks intentional
- consistent cap height
- consistent stroke thickness
- consistent inner counters
- predictable side bearings once converted into a real font

### What We Are Actually Building

This pass should target:

- an all-caps display face
- strong enough for title text and primary buttons
- readable at medium-large menu sizes
- visually specific to this game

This pass should not try to solve:

- paragraph text
- leaderboard names
- settings pages
- broad localization coverage

### Font Aesthetic Target

The custom display font should read as:

- ritual
- severe
- worn
- low-fi
- slightly brutal
- not elegant
- not slick
- not fantasy-book clean
- not metal-band unreadable

The best mental model is:

- shrine inscription
- propaganda title card
- damaged cartridge-era key art lettering

### Font Production Plan

1. Generate glyph sheets through the ChatGPT Chrome workflow.
2. Start with grouped uppercase letters, not one giant alphabet prompt.
3. Lock the style before finishing the full set.
4. Once the look is approved, complete the minimum glyph set.
5. Normalize glyph boxes, baseline, spacing, and stroke consistency.
6. Convert the approved set into a real font file later.

Generation grouping recommendation:

- `A-F`
- `G-L`
- `M-R`
- `S-X`
- `Y-Z`, digits, punctuation

Generating four to six letters at a time is acceptable if the prompt hard-locks the style and the previous approved letters are attached as references in later prompts.

### Font Prompt Rules

- all caps only
- flat front-facing glyph presentation
- one color family only during ideation
- no perspective
- no decorative background
- no poster layout
- no extra symbols unless explicitly requested
- no medieval manuscript calligraphy
- no chrome bevel fantasy-logo polish
- no graffiti energy

Request the glyphs as clean presentation sheets on plain dark or plain neutral backgrounds so they are easy to crop and compare.

### Font Evaluation Criteria

Score each attempt from `1-5` on each axis:

- `retro authenticity`
- `cozy containment`
- `threat / brutality`
- `readability`
- `non-generic identity`

Quick interpretation:

- `5` = strong fit
- `3` = partially useful
- `1` = wrong direction

Reject immediately if the output reads as:

- generic fantasy RPG logo
- generic horror poster font
- modern esports UI
- noisy death-metal unreadable
- AI slop with inconsistent letter construction

### Font Pass Log Template

Use this block for every meaningful font attempt:

```md
## Font Pass <ID>

- Date:
- Conversation / run:
- Glyph group:
- Output path:
- Prompt summary:
- Reference images used:
- Retro authenticity: <1-5>
- Cozy containment: <1-5>
- Threat / brutality: <1-5>
- Readability: <1-5>
- Non-generic identity: <1-5>
- Closest so far: Yes/No
- What works:
- What fails:
- Next change:
- Status: Keep / Maybe / Reject
```

## Decision: Background Image

The main menu should keep the large background image and the current broad concept:

- monumental altar
- eclipsed sun
- centered dominant figure or monument
- hostile landscape behind it

What must change is the specificity of the elements.

The current image reads too much like generic AI fantasy poster art. The next passes should keep the composition logic but make each element feel authored and world-specific.

### Background Structure To Preserve

Preserve the current layer logic:

- `sky_bg` = distant environment, mountains, clouds, atmospheric depth
- `fire_moon` = eclipse disk, corona, smoke, fire glow, sky event
- `pyramid_chad` = altar, stairs, foreground idol/monument, primary silhouette

This is important because the code and import workflow already support those three layers cleanly.

### Background Aesthetic Target

The new background should read as:

- infernal monument
- apocalyptic ritual architecture
- darker, harsher world
- low-fi menace
- still iconic and legible behind UI

It should not read as:

- generic fantasy splash art
- high-fantasy golden temple postcard
- polished mobile-game home screen
- noisy gore collage

### Specific Element Corrections

#### Altar / Monument

Current problem:

- too generic
- too clean
- too much "golden fantasy temple"

Target:

- worn basalt, soot, stained brass, or sacrificial-stone language
- stronger silhouette
- less generic ornament, more specific shrine logic
- feels old, oppressive, and used

#### Eclipse / Fire Layer

Current problem:

- visually striking but still reads as generic dramatic backdrop

Target:

- harsher corona
- more ominous eclipse event
- heavier smoke, ash, or atmospheric contamination
- more unnatural celestial behavior

#### Background Landscape

Current problem:

- generic matte-paint mountains

Target:

- more hostile topography
- more shape rhythm
- less default fantasy landscape
- clearer atmospheric hierarchy so UI still reads in front of it

#### Central Figure / Idol

Current problem:

- reads too much like generic AI handsome hero monument

Target:

- stronger idol or relic read
- more monumental and less portrait-like
- more uncanny or severe
- should feel like a thing the world built around, not just a character poster

### Background Prompt Rules

- preserve the composition
- preserve the centered monumental layout
- preserve space for the main menu title and center CTA stack
- do not add side clutter that competes with panels
- do not solve this with random blood splatter overlays
- do not over-detail the far background
- do not turn the image into a modern polished splash-art poster

### Background Evaluation Criteria

Score each attempt from `1-5` on each axis:

- `world specificity`
- `menace`
- `cozy containment`
- `retro compatibility`
- `UI compatibility`

Interpretation:

- `world specificity` means it feels like this game's world, not generic AI fantasy
- `cozy containment` means the art feels visually bounded and usable behind UI
- `retro compatibility` means it could survive PS1-adjacent treatment without falling apart
- `UI compatibility` means the menu can still sit over it without becoming unreadable

### Background Pass Log Template

Use this block for every meaningful background attempt:

```md
## Background Pass <ID>

- Date:
- Conversation / run:
- Layer target: sky_bg / fire_moon / pyramid_chad / composite preview
- Output path:
- Prompt summary:
- Reference images used:
- World specificity: <1-5>
- Menace: <1-5>
- Cozy containment: <1-5>
- Retro compatibility: <1-5>
- UI compatibility: <1-5>
- Closest so far: Yes/No
- What works:
- What fails:
- Next change:
- Status: Keep / Maybe / Reject
```

## Recommended Working Order

1. Lock the custom display-font direction first.
2. Do not attempt to finish a universal UI font family in this pass.
3. Create one composite background preview that proves the new material direction.
4. Once that direction is approved, split or regenerate to match the three production layers.
5. Only after font and background are directionally approved should panel and border work begin.

## Prompt Scaffolds

These are starting points, not final prompts.

### Custom Display Font Scaffold

```text
Create a clean glyph presentation sheet for a custom all-caps display font for a dark indie retro action game UI.

Show only these glyphs: A, B, C, D, E, F.

Style target: ritual shrine inscription, brutal but readable, worn edges, low-fi PS1-era mood, slightly gory atmosphere without literal blood dripping, severe geometry, compact and self-contained, not elegant, not modern, not glossy, not medieval calligraphy, not death-metal unreadable.

Presentation rules: flat front-facing letters, consistent cap height, consistent stroke thickness, plain dark background, no perspective, no poster layout, no extra symbols, no decorative frame.
```

### Background Composite Scaffold

```text
Create a main menu background concept for a dark indie retro action game.

Keep the same overall composition concept: a monumental altar or ritual monument in the center foreground, an eclipsed sun directly behind it, a hostile mountain landscape in the distance, and clear central negative space for a game title and menu buttons.

Art direction: infernal shrine, apocalyptic ritual architecture, worn basalt and stained brass materials, ominous celestial event, darker and more authored than generic fantasy art, PS1-compatible low-fi mood, severe and iconic, slightly brutal, slightly gory in tone but not messy, visually contained enough to work behind UI.

Avoid: generic fantasy poster art, clean golden temple look, handsome hero portrait vibe, modern polished splash-art lighting, random side clutter, excessive detail that competes with UI.
```

## Future-Model Pass Instructions

When GPT-5.5 or a newer image model is available, do not restart this exercise from scratch.

Use this order:

1. Read this file.
2. Read [ChatGPT_Image_Production_Workflow.md](/C:/UE/T66/Docs/Art/ChatGPT_Image_Production_Workflow.md).
3. Reuse the approved references from this pass.
4. Keep the scoring system and pass log format.
5. Improve only one major variable at a time.

The later pass should specifically try to improve:

- glyph consistency across the custom display font
- world specificity in the background
- monument/material believability
- UI-readability under retro degradation

## Current Summary Decision

As of this preliminary pass:

- the custom-font route is approved for the display face
- the custom-font route is not yet approved for full utility/body text
- the main menu background concept is approved
- the current background execution is not approved
- the three-layer main menu background structure should remain intact
- panel and border redesign should wait until font and background direction are settled

## 2026-04-10 Exploration Batch

This batch was generated through the local Chrome ChatGPT bridge workflow using fresh ChatGPT tab generation per request.

Run rules used:

- fresh generation request per image to reduce duplicate-looking outputs
- broad style branching instead of minor prompt nudges
- `GO!` used as the display-font evaluation word
- altar concepts generated as isolated object concepts only

Result summary:

- `5` `GO!` sheet images generated
- `20` total `GO!` variants available for review
- `10` altar concept images generated
- all generated PNG files in this batch had unique SHA256 hashes

Primary review locations:

- [go-sheets-contact.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/go-sheets-contact.png)
- [altars-contact.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/altars-contact.png)

Raw output roots:

- `C:\UE\T66\SourceAssets\UI\MainMenu\StyleExploration\Fonts\GO_Sheets`
- `C:\UE\T66\SourceAssets\UI\MainMenu\StyleExploration\Altars`

### GO! Sheets

- `Attempt_01`: chipped basalt shrine inscription
- `Attempt_02`: rusted sacrificial iron signage
- `Attempt_03`: bone reliquary inscription
- `Attempt_04`: tarnished brass liturgical plaque
- `Attempt_05`: charred wood and dried-blood lacquer block

Files:

- [go-sheet-attempt-01.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/GO_Sheets/Attempt_01/go-sheet-attempt-01.png)
- [go-sheet-attempt-02.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/GO_Sheets/Attempt_02/go-sheet-attempt-02.png)
- [go-sheet-attempt-03.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/GO_Sheets/Attempt_03/go-sheet-attempt-03.png)
- [go-sheet-attempt-04.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/GO_Sheets/Attempt_04/go-sheet-attempt-04.png)
- [go-sheet-attempt-05.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/GO_Sheets/Attempt_05/go-sheet-attempt-05.png)

Current status:

- review pending
- no winners selected yet

### Altars

- `Attempt_01`: stepped basalt sacrificial pyramid
- `Attempt_02`: rusted iron industrial execution altar
- `Attempt_03`: bone reliquary altar
- `Attempt_04`: tarnished brass and obsidian liturgical altar
- `Attempt_05`: jagged volcanic blackstone altar
- `Attempt_06`: brutalist concrete monolith altar
- `Attempt_07`: eroded ancient temple altar
- `Attempt_08`: cathedral-machine hybrid altar
- `Attempt_09`: coffin-shrine altar
- `Attempt_10`: ash-stained butcher chapel altar

Files:

- [altar-attempt-01.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_01/altar-attempt-01.png)
- [altar-attempt-02.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_02/altar-attempt-02.png)
- [altar-attempt-03.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_03/altar-attempt-03.png)
- [altar-attempt-04.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_04/altar-attempt-04.png)
- [altar-attempt-05.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_05/altar-attempt-05.png)
- [altar-attempt-06.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_06/altar-attempt-06.png)
- [altar-attempt-07.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_07/altar-attempt-07.png)
- [altar-attempt-08.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_08/altar-attempt-08.png)
- [altar-attempt-09.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_09/altar-attempt-09.png)
- [altar-attempt-10.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars/Attempt_10/altar-attempt-10.png)

Current status:

- review pending
- no winners selected yet

## 2026-04-10 Follow-up Batch

This follow-up batch recorded two new decisions from user review:

- preferred base font direction for follow-up exploration: `GO02`
- altar redo requirement: every altar attempt must visibly include a giant Chad skull or Chad face on top

### Continue Font Batch

Goal:

- derive a new set from `GO02`
- replace `GO!` with `CONTINUE`
- keep the angular religious-brutalist shape language
- preserve the `cross inside the O` motif
- move away from orange rust
- push toward blood-red, crimson, oxblood, and sacrificial-red undertones
- use solid black backgrounds only

Review sheet:

- [continue-go02-contact.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/continue-go02-contact.png)

Raw outputs:

- [Continue_From_GO02](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02)

Files:

- [continue-sheet-attempt-01.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02/Attempt_01/continue-sheet-attempt-01.png)
- [continue-sheet-attempt-02.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02/Attempt_02/continue-sheet-attempt-02.png)
- [continue-sheet-attempt-03.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02/Attempt_03/continue-sheet-attempt-03.png)
- [continue-sheet-attempt-04.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02/Attempt_04/continue-sheet-attempt-04.png)
- [continue-sheet-attempt-05.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Fonts/Continue_From_GO02/Attempt_05/continue-sheet-attempt-05.png)

Current status:

- review pending
- user-selected ancestor direction was `GO02`

### Chad Skull Altars

Goal:

- redo the altar batch so the monument is no longer just an altar
- every attempt must clearly include a giant Chad skull or Chad idol face on top
- include some metallic or golden options and some non-metal options

Review sheet:

- [altars-gigachad-contact.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/altars-gigachad-contact.png)

Raw outputs:

- [Altars_GigaChad](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad)

Files:

- [altar-gigachad-attempt-01.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_01/altar-gigachad-attempt-01.png)
- [altar-gigachad-attempt-02.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_02/altar-gigachad-attempt-02.png)
- [altar-gigachad-attempt-03.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_03/altar-gigachad-attempt-03.png)
- [altar-gigachad-attempt-04.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_04/altar-gigachad-attempt-04.png)
- [altar-gigachad-attempt-05.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_05/altar-gigachad-attempt-05.png)
- [altar-gigachad-attempt-06.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_06/altar-gigachad-attempt-06.png)
- [altar-gigachad-attempt-07.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_07/altar-gigachad-attempt-07.png)
- [altar-gigachad-attempt-08.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_08/altar-gigachad-attempt-08.png)
- [altar-gigachad-attempt-09.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_09/altar-gigachad-attempt-09.png)
- [altar-gigachad-attempt-10.png](/C:/UE/T66/SourceAssets/UI/MainMenu/StyleExploration/Altars_GigaChad/Attempt_10/altar-gigachad-attempt-10.png)

Validation note:

- visual review completed on the contact sheet
- all `10` attempts visibly include a large Chad skull or Chad face as the top focal element

Current status:

- review pending
- skull-presence rule satisfied for the whole batch
