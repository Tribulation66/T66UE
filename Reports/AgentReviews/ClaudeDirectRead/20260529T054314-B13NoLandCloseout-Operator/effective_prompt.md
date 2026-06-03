You are Claude acting as the full-access T66 Operator.

Codex approval artifact: C:\UE\T66\Reports\AgentReviews\20260529_B13_NoLand_Closeout\codex_operator_approval.md

Codex has approved you to make changes inside the approved task contract and scope. You may use the normal Claude Code tool surface available in this environment, including file edits, shell commands, and configured MCP/editor tools such as Blender or other available MCP servers, when they are needed for the approved task.

You must stop and report Codex Approval Required: before any material scope expansion, destructive operation, credential or billing change, git commit, git push, git tag, git reset, git clean, broad Git/LFS scan over Unreal binary asset folders, or any action that contradicts AGENTS.md or folder-owned instructions. If you are unsure whether an action is inside the approved scope, stop and request Codex approval instead of doing it.

Your output is an Operator work artifact and is not a greenlight. Codex will validate your actual changes, run or review verification, and write the final user-facing report.
# Claude Operator Prompt: B.13 No-Land Closeout

Working task:
Formally close B.13 as a no-land by consolidating findings into one authoritative audit file, updating standing docs, verifying the live repo has no B.13 renderer work present, and reporting isolated worktree disposition.

Operator:
Claude

Validator:
Codex

Scope:
Documentation and verification only. Create/update Markdown/report artifacts as needed. Run non-destructive verification commands. No live runtime renderer changes, no B.13R work, no asset deletion, no isolated-worktree deletion, no Git commit/push/reset/clean.

Stop condition:
The live repo contains a single authoritative B.13 no-land audit, standing docs point to it, live source/content has been checked for B.13 leftovers, build/run proof is recorded or a precise blocker is documented, and `C:\UE\T66_B13_Worktree` disposition is reported without deleting it.

User decision to record:
- B.13 is closed as no-land.
- Instanced rendering (HISM and ISM) empirically regresses full-resolution performance for constantly-moving VAT mobs in UE 5.7; every variant tested lost to the existing per-mob static-mesh renderer.
- The per-mob static-mesh-component renderer is the chosen, deliberate renderer for basic mobs, not a placeholder awaiting instancing.
- A GPU-driven crowd renderer, manager-owned position/frame data in a buffer or texture read in the vertex shader while bypassing UE component instance transforms, is the only direction that could plausibly beat the current path. It is deferred and should be revisited only if enemy counts exceed the design ceiling and rendering becomes the measured bottleneck.

Applicable instructions:
- Read `C:\UE\T66\AGENTS.md`.
- Read `C:\UE\T66\OPERATOR_VALIDATOR_PROTOCOL.md`.
- Read `C:\UE\T66\PerformanceSystem\PERFORMANCE_SYSTEM_AGENTS.md`.
- Read `C:\UE\T66\Reports\AGENTS.md`.
- Read `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md` before editing docs.
- Default Mini/minigame exclusion applies.

Required work:

1. Create the authoritative audit file, recommended path:
   `C:\UE\T66\PerformanceSystem\B13_MobInstancedRendering_Audit.md`

   It must consolidate:
   - attempted working goal and staged de-risk approach;
   - empirical full-resolution before baseline: median `189.65`, 1% low `156.16`, 0.1% low `72.03`;
   - candidate table for HISM frame-only, ISM feasibility, four-slot world-offset, transform-cache, hidden-pool, render-flag, spatial-cell at 2000uu and 500uu, and GPU-render-flag probes;
   - root cause: per-frame instance transform updates plus HISM tree management cost more than draw-call reduction saves; ISM removes hierarchy cost but still loses; functionally correct but performance-negative for moving mobs;
   - UE 5.7 engine-source audit finding that HISM tree rebuild/update cost cannot be disabled for moving instances;
   - final decision: no-land, per-mob static mesh renderer chosen, GPU-crowd escape hatch deferred.

2. Point existing scattered artifacts at the audit:
   - Update `C:\UE\T66\PerformanceSystem\2026-05-23_T66_LightweightActor_Plan.md` so B.13 reads closed/no-land and points to the new audit.
   - Update `C:\UE\T66\PerformanceSystem\pending_issues_PerformanceSystem.md` so the dynamic mob HISM item is closed/no-land, not pending, and points to the new audit.
   - If there are obvious B.13 combined packet references that should point forward, update docs only. Do not rewrite historical evidence.

3. Verify live repo is clean of B.13 renderer work:
   - Confirm `C:\UE\T66` live source uses per-mob static-mesh-component rendering and none of the isolated HISM/ISM renderer changes are present.
   - Specifically check whether `M_EasyMobVAT_Unlit_UV2_Instanced` or other B.13-created instanced materials exist anywhere in live `Content`. Report yes/no. If present, flag as Pass D orphan for later removal; do not delete it.
   - Confirm no half-applied renderer change, dangling proof/automation hook, or orphaned reference to B.13 instanced material remains in live source.
   - Confirm the camera-angle invisibility bug is absent from live because no HISM renderer landed. Use source/content evidence and, if feasible, runtime/smoke evidence.
   - Confirm live build compiles and runs on the per-mob renderer. If a build/run is blocked by external state, document the precise blocker and the command used.

4. Report isolated worktree disposition:
   - Inspect/report what `C:\UE\T66_B13_Worktree` contains.
   - Confirm B.13 evidence from that worktree is already preserved in live repo `Reports/` and the new audit; if anything valuable exists only in the worktree, copy the documentation/proof reference into live Reports/PerformanceSystem before recommending deletion.
   - Recommend deletion only after evidence is safe. Do not delete the worktree.

5. Produce an Operator completion artifact at:
   `C:\UE\T66\Reports\AgentReviews\20260529_B13_NoLand_Closeout\operator_completion.md`

   Include:
   - files changed;
   - exact verification commands and results;
   - live renderer/source/content findings;
   - worktree disposition;
   - unresolved caveats;
   - token routing metadata if available.

Out of scope:
- Any live renderer change.
- Pursuing B.13R or GPU-crowd implementation.
- Deleting the isolated worktree.
- Deleting instanced material assets if found.
- Enemy roster review / Pass C.
- Git commit, push, reset, clean, or branch operations.

Important:
- Do not use broad `git status` or `git diff` over content. Use narrow file checks and filesystem/source searches.
- Do not inspect Mini/minigame paths.
- Preserve user-owned dirty work.

