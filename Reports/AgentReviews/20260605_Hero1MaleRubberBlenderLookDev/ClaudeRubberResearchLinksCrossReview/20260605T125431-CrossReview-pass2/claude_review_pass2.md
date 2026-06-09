Result: OK

## Summary
Codex delivered the requested structure (curated YouTube + non-YouTube list, plus a preliminary failure hypothesis) and the failure hypothesis is well-aimed (clearcoat/gloss/specular driving a resin read). But the link list has two real problems Codex can fix without the user: (1) the URLs appear unverified and several specific video IDs are high hallucination risk, and (2) a chunk of the list points at glossy plastic / vinyl-toy looks — the exact "resin on top" look the user is rejecting. My independent pass went NEEDS_USER because it assumed WebSearch was the only path and was denied; however Codex can verify candidate URLs with WebFetch and prune the bad ones itself, so this stays OK.

## Suggested Answer Patch
- Before sending, have Codex WebFetch each URL and drop any that 404 or redirect to something off-topic. Do not publish a link the agent could not resolve this turn.
- Re-balance toward the actual target (matte/satin soft-toy rubber, Fall Guys / vinyl squishy look) and away from glossy/plastic:
  - Likely keep: Kirby stylized material (TooEazyCG), the dedicated rubber-shader tutorials, the rubber-material article, the Principled BSDF manual page (coat/specular/roughness reference).
  - Likely cut or demote as off-target: "Basic Glossy Shader," "Glossy Plastic … BSDF," "Vinyl Toy Textures with Octane" (glossy resin-like reads — the opposite of the request).
- Add a one-line "why relevant" per source (the prompt explicitly asked for this and the draft omits it).

## Issues To Fix
- **Unverified URLs / hallucination risk.** Specific YouTube video IDs and the niche article/Stack Exchange links are exactly the kind of thing fabricated from memory. The user's next action is spending effort pulling transcripts, so a dead/wrong link is actively costly. Verify each before emitting.
- **Scope drift toward gloss/plastic.** Several entries chase the look the user said failed ("varying degrees of resin"). Prune or clearly label them as "what NOT to do" examples rather than targets.
- **Missing the stated emphasis.** Working-task scope calls out "soft toy / Fall Guys-style rubber" and subsurface/sheen; the draft is heavy on generic rubber/plastic shading and light on stylized soft-toy + SSS/sheen sources.
- **No per-source rationale.** Prompt asked for "why each source is relevant"; add it.

## Question For User
None required — Codex can verify and prune links itself via WebFetch and re-curate toward the soft-toy target. (If WebFetch is also unavailable to Codex at runtime, then it becomes a user unblock — but don't assume that yet.)

## Evidence Or Verification Gaps
- The draft presents no evidence any link was opened/verified this turn. Treat all as unverified until WebFetch-confirmed.
- Failure hypothesis is inferred from the "looks like resin" description, not from inspecting the actual material graph (correct for research-only scope) — keep it labeled preliminary, as the draft does.

## Notes
- The failure hypothesis is solid and aligns with my independent diagnosis (kill/reduce coat, raise roughness, lower specular/f0, add roughness/normal microvariation, soft Fresnel sheen, shallow SSS). Good to keep.
- Honoring the stop condition: list + preliminary hypothesis only; full postmortem waits for transcripts. Draft does this correctly.
