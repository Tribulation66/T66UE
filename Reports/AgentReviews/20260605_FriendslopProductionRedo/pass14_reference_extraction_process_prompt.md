# Pass14 Corrected Process Prompt

Original handoff: `C:\UE\T66\Reports\AgentReviews\20260605_FriendslopProductionRedo\fresh_agent_main_menu_pass14_prompt_final.md`

Current user correction:

```text
Im already stopping you, because I can already tell these UI elements youre making do not match the reference image, we need a better process to extract the UI elements so they look exactly like the UI elements that are in the reference image but without the text or content, and then a stronger gate to validate the UI elements produced, so we dont get this problem what do you reccomend?
```

Codex recommendation already given:

```text
Use reference-first extraction rather than freeform imagegen; preserve exact reference silhouettes/material/lighting; remove only content holes; use account-backed imagegen only as masked inpaint/edit if available; add per-component gates comparing reference crop, mask, cleaned blank plate, alpha/checker preview, difference overlay, silhouette/edge/material checks, and manual visual PASS before runtime wiring.
```

Current continuation request:

```text
Ok continue with the solutions
```

Task for Validator:

Provide an independent repo-grounded answer for continuing pass14 after this correction. Check the T66/Friendslop process constraints and identify the safest implementation path. Assume Codex is Operator and Claude is Validator. Key question: should pass14 proceed by restoring the generic generated plates and creating reference-derived component extraction/gating artifacts before any runtime wiring? Note any mandatory stop conditions if the built-in account-backed imagegen tool cannot perform local-image/mask editing and CLI/API fallback is forbidden.
