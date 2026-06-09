You are Claude cross-reviewing a Codex draft for the T66 Unreal project.

Rules:
- Include a clear Result: OK or Result: NEEDS_USER line near the top.
- Prefer starting with the result line, but the parser will also accept a clear
  result line or unambiguous OK / needs-user meaning elsewhere in the response.
- Do not edit files.
- Do not run mutating commands.
- Treat Codex as the Operator/final router and you as the Validator.
- Compare the original prompt, Codex draft, and your independent answer when present.
- Look specifically for mistakes, missed constraints, risky assumptions, weak evidence, scope problems, and unclear wording.
- Patch the answer text when the fix is straightforward.
- Return concrete issues when Codex needs to inspect, edit, verify, or ask the user before answering.
- Ask a user question only when the user is the only person who can decide the next path.
- Keep the review concise and practical. Do not create packet-completeness ceremony or hard review-depth categories.

Your result should be one of these two lines:
Result: OK
Result: NEEDS_USER

After that result line, return a concise Markdown review with exactly these headings:
Summary
Suggested Answer Patch
Issues To Fix
Question For User
Evidence Or Verification Gaps
Notes

Result meanings:
- OK: the models can handle the prompt internally. You may still list corrections, evidence gaps, or wording patches for Codex to handle before answering.
- NEEDS_USER: the user's attention is required because only the user can decide, approve, unblock a missing prerequisite, resolve an unavailable required tool, or change the scope.

Do not use NEEDS_USER for ordinary mistakes or missing edits that Codex can fix. List those inside the review body and keep the result OK.

Review scope:
- Original prompt path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\rubber_research_links_prompt.md
- Codex draft path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\rubber_research_links_draft.md
- Independent answer path: C:\UE\T66\Reports\AgentReviews\20260605_Hero1MaleRubberBlenderLookDev\ClaudeRubberResearchLinks\20260605T125325-IndependentAnswer-pass1\claude_review_pass1.md
- Output scope: targeted cross-review and answer patch only.

<original_prompt>
Original user request:
Ok so I took a look at it and none of them actually looked how I wanted it to look they all just look like they have varying degrees of resin on top of them, do some research and find some youtube videos that discuss how to get the look that the model is made of rubber. As well as other non youube video sources and then give me the links I'll get the transcripts for you and after sending you the transcripts you need to tell me why you were wrong in this last pass.

Working task:
Operator: Codex
Validator: Claude
Scope: Research credible YouTube and non-YouTube sources on making a model read as rubber, especially soft toy/Fall Guys-style rubber, and return links for transcript collection. No Blender/Unreal changes in this turn.
Stop condition: Provide a curated source list with why each source is relevant, plus only a preliminary likely failure mode of the last pass. Full postmortem waits for transcripts.


</original_prompt>

<validator_independent_answer>
Result: NEEDS_USER

## Independent Answer

The core deliverable here — a curated list of *real, verifiable* YouTube and non-YouTube links for the user to pull transcripts from — cannot be produced reliably right now. WebSearch is the required tool for this and it returned a permissions error (not granted). Per this project's hard rule, I must not guess or fabricate URLs, and fabricated links are actively harmful here because the user's next step is spending effort fetching transcripts from them. So the link list is blocked on a tool the user must enable (or run themselves).

What Codex/Claude *can* deliver internally without web access — and should put in the answer body so the turn isn't empty:

**Preliminary likely failure mode of the last pass (the requested "why it looked like resin, not rubber"):**
The "varying degrees of resin on top" symptom points to a clearcoat/specular problem, not a base-color problem:
- A clearcoat (or high specular + low specular-roughness) layer was almost certainly left on or too sharp, producing a glossy wet film that reads as resin/epoxy.
- Reflection roughness was too low and too uniform — real rubber has a broad, soft, slightly-matte highlight, not a tight specular hotspot.
- Likely missing: diffuse/microsurface roughness variation (rubber is matte-to-satin with subtle roughness breakup), a soft sheen/fresnel rim rather than a hard rim, and a gentle, shallow subsurface for the soft-toy/Fall-Guys read.
- Base diffuse roughness was probably too low overall.

This is preliminary only; the full postmortem correctly waits for transcripts.

**To unblock the link list, the user has two clean paths** (and a search-term starter kit Codex can hand over so the user can self-search): search terms like "rubber material Blender shader breakdown," "Fall Guys / soft toy stylized rubber shader," "vinyl toy / squishy rubber Substance material," "rubber clearcoat off matte sheen." Likely-relevant venues to target: Blender shader-tutorial channels, ArtStation material breakdowns, Polycount threads, and Substance/Adobe material docs. (Channel/creator names from memory are not reliable enough to publish as links — verification needed.)

## Evidence Checked
- Original prompt and working-task scope (research-only, no Blender/Unreal changes, stop at curated source list + preliminary failure mode).
- This project's standing rule: never generate or guess URLs unless confident they aid programming.
- WebSearch availability: loaded but **permission denied** at call time — confirmed, not assumed.

## Questions Or Blockers
- **WebSearch is not permitted.** Producing verified links requires it. Do you want to (a) grant WebSearch permission so the agents can gather and verify real source links, or (b) run the searches yourself using the search-term kit above and paste back the links/transcripts? Only you can unblock this.

## Caveats
- The failure-mode diagnosis is inferred from the "looks like resin" description, not from inspecting the actual material graph this turn (scope is research-only). Treat it as a hypothesis to confirm against transcripts.
- Do not let Codex emit any specific YouTube/article URLs from memory — without WebSearch verification they risk being hallucinated, which directly wastes the user's transcript-collection effort.
- If WebSearch stays unavailable, the turn's realistic output is the failure-mode hypothesis + search-term kit, not a link list.

</validator_independent_answer>

<codex_draft>
Draft source list:

YouTube candidates:
1. Blender Rubber Material Shader using Principled BSDF Texture Node — blenderian — https://www.youtube.com/watch?v=8LqN2Yy55AY
2. Blender Cycles Rubber Shader and Node Group Tutorial — Jayanam — https://www.youtube.com/watch?v=vngGTaBCZ-o
3. Basic Glossy Shader in Blender 2.8 - Materials Tutorial (Eevee) — chocofur — https://www.youtube.com/watch?v=VrsteZ3Ci3w
4. Achieving Kirby's Stylized Material in Blender [Shader Tutorial] — TooEazyCG — https://www.youtube.com/watch?v=ZatjXklv8No
5. How to make Blender Glossy Plastic and Matte Plastic Material using BSDF shader — blenderian — https://www.youtube.com/watch?v=zxvjZwg7JeI
6. Cinema 4D Tutorial - Creating Plastic Vinyl Toy Textures with Octane Render — eyedesyn — https://www.youtube.com/watch?v=8Njl5hLmGs0

Non-YouTube sources:
1. Blender Manual: Principled BSDF — https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/principled.html
2. Blender Stack Exchange: How to Create a Vinyl Plastic Toy Shader? — https://blender.stackexchange.com/questions/116195/how-to-create-a-vinyl-plastic-toy-shader?noredirect=1
3. Blender Stack Exchange: grainy rough plastic material — https://blender.stackexchange.com/questions/142447/how-can-i-create-a-grainy-rough-plastic-material-in-blender-just-like-one-on-mi/142456#142456
4. CGian: Blender Black Rubber Material — https://cgian.com/blender-rubber-plastic-material/
5. Blender 4.0 Shading & Texturing release notes — https://developer.blender.org/docs/release_notes/4.0/shading/
6. PlayCanvas PBR physical rendering guide — https://developer.playcanvas.com/user-manual/graphics/physical-rendering/
7. Open 3D Engine PBR material documentation — https://www.docs.o3de.org/docs/atom-guide/look-dev/materials/pbr/
8. Roughness Setting reference — https://sameerbaloch.com/roughness-setting/

Preliminary failure hypothesis:
The prior pass likely varied clearcoat/specular/gloss too aggressively and used coat as the primary readability control, so it created resin/plastic topcoat highlights rather than soft rubber. The next pass should focus on higher roughness, lower or zero clearcoat, lower specular/f0, roughness/normal microvariation, soft Fresnel/sheens, flattened albedo, and geometry silhouette.


</codex_draft>
