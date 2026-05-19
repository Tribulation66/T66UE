# Pending Issues - Materials

## View-Space Lit Character Master Needs Visual Lock Decision

- Severity tag: [Minor]
- What's wrong: `/Game/Materials/M_GLB_ViewSpaceLit_Character` is still retained as the Track 2 master material for future A/B testing, while the production mob path currently uses `/Game/Materials/MI_GLB_Unlit_Character_Shared`. The Track 2 test instances were archived, but the master remains because the visual lock has not decided whether view-space cel lighting should replace or supplement the active unlit path.
- Why it's out of scope now: This cleanup pass only archived unused/test assets and reduced warning noise. It did not run the visual A/B required to choose the final character lighting model.
- What fixing it would entail: Run a visual-lock comparison between the active unlit shared material and the view-space lit master on the production mob set, then either promote the master into the runtime path or archive it with a rollback note.
