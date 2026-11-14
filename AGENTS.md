# Project agent workflow

Read `CLAUDE.md` and the current project status and release plan before working.

## Delegate routine work

Dylan requests subagents wherever useful in this project. Delegate simpler,
bounded tasks such as running tests, reviewing logs, checking diffs, and
read-only investigation. The primary agent owns implementation decisions,
integration, and the final report. Give each subagent a concrete task and ask
for evidence and limitations, not just a pass/fail assertion.

Use `gpt-6-luna` for these routine tasks to reduce token cost. Spawn it with
`fork_turns: "none"` and a concise, self-contained brief: workspace, exact
commands, expected evidence, resource ownership, and relevant constraints.
Use a larger model only when the task needs deeper reasoning or Luna reports
a problem it cannot resolve. Do not default routine verification to Terra or
the primary agent's inherited model. The primary agent should summarize the
results instead of repeating the checks without a concrete reason.

Coordinate shared resources: only one Unreal build, editor, game verification,
or automation run at a time. A verification subagent may run Unreal when the
primary agent explicitly hands it that responsibility. Do not let multiple
agents edit the same files; assign distinct ownership if delegating edits.
Inspect subagent results and resolve failures before committing.

Use independent routine tasks in parallel when practical; keep dependencies
and engine work sequential. Do not spawn agents just to duplicate completed work.
