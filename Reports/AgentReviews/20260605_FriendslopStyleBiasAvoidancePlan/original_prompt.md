User prompt:
Ok the issue here is that, the styles were not really close to the game references at all, and ended up drifting to this kind of default UI that imagegen has a strong bias too. So I know you said you wanted to avoid language specifically naming the games to avoid copywright but none of this will be published its just vision boarding about direction, and we need the references to be closer to the games I mentioned. And we can actually get rid of the Third one which i think is lethal company and switch to fall guys. But its very important that before you send them to me all 4 look unique from each other,like trully unique with a high degree of variance. And especially except for the 3 image, I can tell that these were made in succession because we also have the drift and bias, of subsequent images referencing previous creation a lot, like for example the Chadpocalypse Title has the same bend for images 1,2 and 4 so for each image generationd eploy a unique codex CLI to generate that specific image. Let me know if you and claude have any other thoughts on how to avoid imagegen bias, be clsoer to reference games, and ensure uniqueness between images.

Working task:
Operator: Codex
Validator: Claude
Scope: Re-evaluate the FriendslopStyle reference-generation process after Round02 drift: closer source-game vision-board references are acceptable, replace Lethal Company with Fall Guys, avoid imagegen default bias and cross-image drift, and ensure each candidate is truly distinct. No image generation yet.
Stop condition: Provide Codex/Claude recommendations for the next generation pass, including process changes, prompt strategy, isolation strategy, and uniqueness checks.

Relevant repo/process context:
- Current generated outputs are in `UI/FriendslopStyle/Reference/MainMenu/Round02`.
- Round02 preserved layout but many candidates share the same arced `Chadpocalypse` title and default chunky fantasy UI bias.
- User wants future generation to keep T66 Main Menu layout/content but be much closer to source-game UI visual language for vision boarding.
- Replace the Lethal Company reference pole with Fall Guys.
- Source references for this vision board are now PEAK, Schedule I, Fall Guys, Gamble With Your Friends, and R.E.P.O.
- User explicitly wants unique Codex CLI / isolated generation per image to reduce cross-contamination and sequential drift.
- These are internal vision-board references, not published assets and not runtime UI assets.

Question for Claude:
- How should the next generation pass be changed to reduce imagegen house-style bias, get closer to source-game UI references, and ensure high variance?
- What should Codex tell the user before generating again?
- Do you agree with isolated per-image Codex workers? If so, what isolation/QA gates should be used?
