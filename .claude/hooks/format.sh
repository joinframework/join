#!/usr/bin/env bash
re='"file_path": *"([^"]*\.(hpp|cpp))"'
sre='"session_id": *"([^"]*)"'
in=$(cat)
[[ $in =~ $sre ]] && mark=${TMPDIR:-/tmp}/format-hook-${BASH_REMATCH[1]}
[[ $in =~ $re ]] || exit 0
file=${BASH_REMATCH[1]}
[[ $file == "$CLAUDE_PROJECT_DIR"/* ]] || exit 0
v=$(sed -n "s/.*clang-format-version: *'\([0-9]*\)'.*/\1/p" "$CLAUDE_PROJECT_DIR"/.github/workflows/build.yml | head -1)
cf=clang-format-$v
command -v "$cf" >/dev/null || cf=clang-format
command -v "$cf" >/dev/null || { echo "format hook: clang-format is missing, nothing was formatted. Ask the user to install it (AGENTS.md, Definition of done)." >&2; exit 2; }
[ -f "$file" ] || exit 0
"$cf" -i "$file" || { echo "format hook: $cf failed on $file" >&2; exit 2; }
have=$("$cf" --version | sed -n 's/.*version \([0-9]*\)\..*/\1/p')
[ -z "$v" ] || [ "$have" = "$v" ] || [ -e "$mark" ] || { touch "$mark"; echo "{\"hookSpecificOutput\":{\"hookEventName\":\"PostToolUse\",\"additionalContext\":\"format hook: formatted with clang-format $have, CI uses $v; ask the user to install clang-format $v.\"}}"; }
exit 0
