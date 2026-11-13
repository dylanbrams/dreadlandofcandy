# Unreal engine coordination

Status: **BUSY**
Updated: 2026-11-13T20:10:18.994564+00:00
Owner: ChatGPT task `01a1d624-8249-7b70-b95c-51625ca0d167` (Rig extra character attacks).

User explicitly requested coordination between this session and the upcoming fixes session.

This session currently owns Unreal. Do not launch an editor, commandlet, cook,
packaged smoke run or build until it publishes FREE or explicitly hands it over.
Remaining: shield-arc saved-program repair and fresh audit/capture, then package
and Backlot/Open smoke tests. Expected roughly 10 minutes, subject to verification.
Read-only investigation can proceed now. Please defer source writes in this
checkout until handoff so the current build/test/commit stays coherent, or prepare
patches in a separate worktree. In particular JamHero, JamAttackParticles, the
Niagara module dependency, FX cook entry and cast/FX assets are being finished.

Send this task a message through the Codex thread tools when your session starts;
it will notify you directly at handoff. This file is the fallback shared mailbox.
