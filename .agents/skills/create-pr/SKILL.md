---
name: create-pr
description: Branch, commit, push and open a PR with the join conventions. Use when asked to commit, push or open a PR.
---

# Branch, commit, push

**Ask the user before each of: creating the branch (propose the name), committing (show files and message), pushing (branch and remote), opening the PR (title and base).** One approval covers one action. Never merge, force-push or push to `main`.

1. **Inspect**: `git status`, `git diff`, current branch. Stage nothing unrelated, nothing under `build/`.
2. **Validate**: the dev loop of `AGENTS.md` "Definition of done" passed; the closing checks run only when the user says the change is ready to merge. Say what was not verified when asking.
3. **Branch**: on `main`, `git switch -c <prefix><kebab-name>`. Stay on a suitable branch; never commit on `main`.
   - `feature/`: additions (class, API, option, tool).
   - `bugfix/`: real bug, root cause fixed, regression test added.
   - `hotfix/`: small quick fix, minimal diff.

   Ask once if bugfix vs hotfix is unclear.
4. **Version**: bump `VERSION` in every PR, unless it is already ahead of the latest tag at an equal or higher level: minor for a feature or an API change, patch otherwise (bugfix, hotfix, tools, CI, docs, agent config), major only on request. Minor or major also sets `PROJECT_NUMBER` = `<major>.<minor>` in `doxyfile`.
5. **Commit**: explicit paths (no `git add -A`), one commit. Message: one capitalized imperative line, about 50 chars, no period, no body, no PR number, no trailer: `Fix netlink dump race`, or `Msgpack binary value type` for a new feature.
6. **Push**: `git push -u origin <branch>`. Never rewrite pushed history; report a rejected push.
7. **PR**: title = commit line, no body. Ask first; with `gh`: `gh pr create --base main --title "<Title>" --body ""`. Otherwise give `https://github.com/joinframework/join/compare/main...<branch>?expand=1`.
8. **Report** branch, commit, presets run and the link; do not claim CI is green.
