#!/usr/bin/env bash
# Namco Museum Megamix (native PC port) launcher.
# Runs the game from its own folder (megamix.cfg, input.cfg and save/ live next to this script) (and the bundled libraries, if the package has a lib/ folder).
# Usage: ./megamix.sh [--data /path/to/files] [other options, see docs/OPTIONS.md or ./megamix.sh --help]
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Paths given on the command line are relative to where the player typed them: make them absolute before changing folder.
args=()
while [ $# -gt 0 ]; do
	case "$1" in
	--data | --dol)
		args+=("$1")
		if [ $# -gt 1 ]; then
			p="$2"
			case "$p" in
			/* | "~"/*) ;;
			*) p="$PWD/$p" ;;
			esac
			args+=("$p")
			shift
		fi
		;;
	*) args+=("$1") ;;
	esac
	shift
done

cd "$here" || exit 1
# A package with its own C runtime (lib/ld-linux-x86-64.so.2): start the game through the bundled loader, so the bundled glibc is used
# even where the system's is older. The OpenGL libraries / drivers still come from the system.
loader="$here/lib/ld-linux-x86-64.so.2"
if [ -x "$loader" ] || [ -f "$loader" ]; then
	exec "$loader" --library-path "$here/lib" --argv0 "$here/bin/megamix" "$here/bin/megamix" "${args[@]}"
fi
[ -d "$here/lib" ] && export LD_LIBRARY_PATH="$here/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"   # --system-libs packages
exec "$here/bin/megamix" "${args[@]}"
