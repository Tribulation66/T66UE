# Mesh Approval Checklist

Use this checklist before approving a raw TRELLIS mesh for retopo or equipment work.

## Pass Criteria

- silhouette reads correctly from the front view
- torso proportions are close enough to the benchmark character
- arms are intact and do not contain major hallucinated appendages
- hands are readable and not catastrophically melted
- pants silhouette is preserved
- feet are intact enough for later cleanup
- textures are readable enough to support evaluation
- no large floating junk geometry
- no obvious floor/platform contamination from background failure

## Additional Checks For Split Workflows

- neck cutoff is manageable
- head scale is plausible relative to the body
- head render is not effectively empty
- seam integration looks realistically salvageable

## Reject Conditions

Reject the raw mesh immediately if any of these are true:

- missing major body regions
- duplicated or phantom limbs
- severe topology spikes dominating the silhouette
- render is mostly empty or unreadable
- the mesh would require sculpting from scratch instead of cleanup

## Current Best Known Arthur Result

Current best body candidate:

- [Arthur_LegacyDirect_Body_Green_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyDirect_Body_Green_Raw.png)

Current weak result that should not be treated as approved:

- [Arthur_LegacyDirect_Head_Green_Raw.png](C:/UE/T66/Model%20Generation/Runs/Arthur/Renders/Arthur_LegacyDirect_Head_Green_Raw.png)
