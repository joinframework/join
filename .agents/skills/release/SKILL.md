---
name: release
description: Tag and publish a join release from main. Use when asked to release or tag a version.
---

# Release

**Ask the user before creating the tag and again before pushing it.** Never move, delete or re-push an existing tag; never push anything else.

1. **Check**:
   - On `main`, clean tree, up to date: `git fetch origin --tags && git status` and `git rev-parse HEAD origin/main` equal.
   - `VERSION` was bumped by a merged PR (skill `create-pr`), is greater than the latest tag (`git tag -l --sort=-v:refname | head -1`) and `v$(cat VERSION)` does not exist locally or on `origin`.
   - `PROJECT_NUMBER` in `doxyfile` = `<major>.<minor>` of `VERSION`.
   - CI (`cd` workflow) is green on this commit: ask the user, or check with `gh run list --workflow cd.yml --commit $(git rev-parse HEAD)` if available.

   If a check fails, stop and say why.
2. **Tag**: lightweight tag on `HEAD` (= `origin/main`, the squash-merged PR commit), like the existing ones: `git tag v$(cat VERSION)`.
3. **Push**: `git push origin v$(cat VERSION)`. `release.yml` checks the tag against `VERSION`, builds (scan, tsan, asan, pack) and publishes the GitHub release with the `.deb` packages and generated notes.
4. **Report** the tag, commit and `https://github.com/joinframework/join/actions/workflows/release.yml`; do not claim the release is published until the user confirms the workflow passed.
