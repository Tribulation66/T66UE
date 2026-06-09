Result: OK

## Independent Answer

**1. Is the concern valid?**

Yes, and the live docs confirm it is already a real failure mode, not a hypothetical. The pass14 addendum exists *because* pass13 produced cropped/baked content failures: `title_not_cropped`, "painted-over center," "fake baked search text," "no baked glyph under live glyph" are all explicit gate rows (pass14 addendum lines 83-93). The title-as-crop example the user cites is precisely the case the addendum's first table row legislates against ("Slate live title, not cropped bitmap").

The risk is that "crop," "alpha extraction," and "contact sheet" are *neutral pipeline verbs* that an agent can reframe as authorization to ship reference pixels. The words themselves are fine; the danger is using them to skip regeneration. So the fix is not to ban the words — the pipeline genuinely needs them (Step E, Step J, Section 8) — but to bind each verb to its *only* legitimate role.

**2. Precise boundary**

Frame it as: crop/alpha/contact-sheet operate on **two non-overlapping object classes** that must never cross.

- **Reference crops** → comparison/gate targets *only*. Allowed: as a visual diff target, as the silhouette/material the candidate is judged against. Forbidden: as, or as the seed pixels of, a runtime plate. (Addendum line 39: "Reference crops are gate targets and comparison artifacts only"; line 47: "do not ship a raw crop/masked lookalike.")
- **Alpha extraction** → allowed only to clean a *blank, regenerated* candidate plate (remove matte, confirm transparency). Forbidden as a way to lift a region out of the full reference into a runtime asset.
- **Contact sheets** → allowed as a verification artifact showing states at min/normal/wide + reference comparison. They are read-only evidence; nothing on a contact sheet is ever imported.

The single discriminator the user should anchor on: **does any production pixel trace back to the reference image?** If yes → fail/stop/regenerate. A runtime plate must originate from a *blank* generation, own only material (silhouette/bevel/gloss/shadow/fill), and contain zero text/glyph/data. The one live exception (2026-06-06 option 1, addendum lines 16-48) is deliberately narrow and screen-scoped to Main Menu pass14, and even it still bans raw crops, baked content, and CLI/API fallback. Future screens revert to regenerate-only.

So the boundary statement is:
> Crop, alpha-clean, and contact-sheet are verification and cleanup verbs. They never author production pixels. Title, icons, glyphs, text, and data are always regenerated clean or owned live by Slate — never lifted, masked, or cropped out of the reference. If you cannot produce a clean blank candidate that passes the component gate, STOP for a user decision; do not ship a crop.

**3. Organization for the 48 files**

The actual problem is that nothing distinguishes the 11-ish current docs from ~36 Round01-06 iteration artifacts, and there is no router. Recommended structure (no content rewrite needed, mostly a README + relocation):

- **`UI/FriendslopStyle/README.md`** (new, router) — names the single process authority, lists active docs by screen, and says explicitly: "Anything under `Reference/<Screen>/Round01-06`, `SourcePrompts/`, and any file named `*passNN*`, `*clean_sheet*`, `*inpaint*`, `*reference_inpaint*` is iteration history. Do NOT treat it as a current rule."
- **Central process doc** — `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` is already the de-facto authority and should be declared the *single source of truth* in its own header and in the README. The user's "one central UI process file" already exists; it just needs to be labeled as such.
- **Active / current docs** — keep at their current paths but list them in the README as the canonical set: implementation instructions, asset registry, current checklist + scorecard template, current element manifest, round06 production plate plan, pass13/pass14 contracts, geometry table, round06 production slice specs.
- **Screen-specific docs** — the README should index by screen (currently only MainMenu), so adding a second screen later doesn't reintroduce ambiguity.
- **Archived / history** — move (or at minimum clearly tag) the Round01-06 prompts/workers/manifests, `clean_sheet`, `inpaint`, `pass11`, `pass12` slice specs into an explicit `Archive/` or `History/` subtree, OR add a one-line banner at the top of each: `STATUS: HISTORICAL ITERATION ARTIFACT — superseded by FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md. Not a current rule.` A physical `Archive/` folder is stronger than banners because agents glob by path.

One caveat on "central" vs the pass13/pass14 contracts: those are *current* and load-bearing, but they are addenda to the central file. The README must make the precedence explicit (central instructions are the spine; pass14 addendum narrows the active Main Menu pass) so an agent doesn't read pass14's narrow exception as the general rule.

**4. Anti-abuse wording**

Add a short, quotable rule block to the central doc and echo it in the README. Suggested text:

> **Provenance rule:** No runtime plate may contain pixels that originated from the reference image. Reference crops are comparison targets only. Alpha extraction cleans blank regenerated candidates only. Contact sheets are evidence only — nothing on one is ever imported.
> **Content rule:** Plates own material only (silhouette, bevel, gloss, shadow, outline, fill). Title, subtitle, labels, glyphs, icons, names, scores, counts, dots, checkboxes, and localization are always Slate-live or regenerated clean. A title is never a crop.
> **Stop rule:** If a clean blank candidate cannot pass the component gate (alpha, no-baked-content, silhouette, material, manual review), STOP for a user decision. Never ship a raw or masked crop. Never fall back to CLI/API/`OPENAI_API_KEY`.

This restates protections that are already scattered across Sections 8, 10, and the addendum ownership rules into one block agents can't misread.

## Evidence Checked

- Glob of `UI/FriendslopStyle/**/*.md` — 47 md files visible (matches the ~48 claim; the 32-doc Reference/MainMenu Round01-06 + SourcePrompts iteration mass confirmed).
- `FRIENDSLOP_STYLE_IMPLEMENTATION_INSTRUCTIONS.md` — already the production authority; Sections 8/10 already ban baked text and reference-as-chrome; no self-declaration as single source of truth yet.
- `Elements/main_menu_pass14_component_contract_addendum.md` — confirms crop/baked-content is a real prior failure; encodes the narrow 2026-06-06 option-1 exception, ownership rules, and component gate.
- Confirmed: no `README.md` and no deprecation/router markers exist (consistent with the prompt's stated repo facts).

## Questions Or Blockers

None that require the user. This is process-analysis with stop condition "recommend structure + organization plan" — fully answerable from current evidence. Codex can deliver this directly; the actual file moves/README creation are a later implementation pass (scope says "no file edits yet").

## Caveats

- I did not open the ~36 iteration docs individually; the active-vs-historical split relies on the prompt's stated facts plus filename patterns. Before any physical `Archive/` move, Codex should confirm none of the Round06 docs are still referenced as live inputs by the central instructions (Section 12 cites Round06 as the pilot reference, so Round06 reference/manifest must stay active even though Round01-05 are history).
- The 2026-06-06 pass14 exception is genuinely live and narrow. Any "regenerate-only, never crop" wording must carve it out explicitly for Main Menu pass14, or the docs will contradict the user's own approved decision.
- File count is 47 vs the stated 48 — a one-file discrepancy (possibly a non-md or a SourcePrompts root file); not material to the recommendation but worth a quick reconcile before declaring the inventory complete.
