#!/usr/bin/env bash
# Builds a release package of the native port: dist/megamix-VERSION-linux-ARCH/ and dist/megamix-VERSION-linux-ARCH.tar.gz.
#
#   tools/make_release.sh [--version V] [--clean] [--jobs N] [--check DATA_DIR] [--system-libs]
#
#   --version V      version string in the package name and README (default: the date, YYYY.MM.DD)
#   --clean          build in a fresh folder (build-release/) instead of reusing build/ incrementally
#   --jobs N         parallel compile jobs (default 2; the build runs under nice -n 19)
#   --no-glibc       do not bundle the C runtime (lib/: glibc + loader, libstdc++, libgcc_s, the X11 helper libraries)
#   --system-libs    old package: link the system raylib / libjpeg-turbo and bundle their .so in lib/
#   --check DIR      after packaging, start the PACKAGED game headless against the game data in DIR (with a throw-away save folder,
#                    from another working directory) and fail if it does not boot
#
# Default = static build (cmake -DMEGAMIX_PORTABLE=ON in build-portable/): raylib and libjpeg-turbo are linked into the binary
# (the script fails if either is still linked dynamically). The C runtime is bundled too (lib/: ld-linux, libc, libm, libstdc++,
# libgcc_s, libX11 / xcb / SM / ICE / GLU ...), and megamix.sh starts the game through the bundled loader, so it runs on systems with
# an older glibc (e.g. a Chromebook's Linux container). Only the OpenGL stack (libOpenGL / libGLX / libGLdispatch + the drivers) comes
# from the system, because it must match the machine's GPU driver.
# The package holds: bin/megamix (stripped), lib/ (only with --system-libs: raylib, libjpeg-turbo), megamix.sh (launcher), megamix.cfg (template),
# README.txt, docs/ (README, OPTIONS, JOBS). It never contains game data: the script refuses to pack main.dol, disc archives,
# BRRES / sound files or saves (the debug font is read from the player's main.dol).
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
version="$(date +%Y.%m.%d)"
clean=0
jobs=2
check=""
portable=1
glibc=1
while [ $# -gt 0 ]; do
	case "$1" in
	--version) version="$2"; shift ;;
	--clean) clean=1 ;;
	--jobs) jobs="$2"; shift ;;
	--check) check="$2"; shift ;;
	--system-libs) portable=0 ;;
	--no-glibc) glibc=0 ;;
	-h | --help) sed -n '2,26p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
	*) echo "make_release: unknown argument '$1' (see --help)" >&2; exit 1 ;;
	esac
	shift
done

arch="$(uname -m)"
name="megamix-$version-linux-$arch"
dist="$root/dist"
out="$dist/$name"
if [ "$portable" = 1 ]; then
	build="$root/build-portable"
	[ "$clean" = 1 ] && build="$root/build-release-portable"
	portflag=ON
else
	build="$root/build"
	[ "$clean" = 1 ] && build="$root/build-release"
	portflag=OFF
fi

echo "== build ($build, RelWithDebInfo = the tested configuration, MEGAMIX_PORTABLE=$portflag, -j$jobs)"
nice -n 19 cmake -S "$root" -B "$build" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMEGAMIX_PORTABLE=$portflag >/dev/null
nice -n 19 cmake --build "$build" -- -j"$jobs"

echo "== stage $out"
rm -rf "$out" "$out.tar.gz"
mkdir -p "$out/bin" "$out/docs"
cp "$build/megamix" "$out/bin/megamix"
strip --strip-unneeded "$out/bin/megamix"

if [ "$portable" = 1 ]; then
	for lib in libraylib libturbojpeg; do
		if ldd "$out/bin/megamix" | grep -q "^[[:space:]]*$lib\.so"; then
			echo "make_release: the portable binary still links $lib dynamically" >&2
			exit 1
		fi
	done
fi

# bundle the C runtime: every library ldd resolves except the OpenGL stack (driver-specific, must stay the system's)
if [ "$glibc" = 1 ]; then
	mkdir -p "$out/lib"
	ldd "$build/megamix" | awk '$3 ~ "^/" { print $1, $3 } $1 ~ "^/" { print $1, $1 }' | while read -r soname path; do
		case "$soname" in
		libOpenGL.so* | libGLX.so* | libGLdispatch.so* | libGL.so* | libEGL.so*) continue ;;
		esac
		cp -L "$path" "$out/lib/$(basename "$soname")"
		echo "   bundled $(basename "$soname")"
	done
	[ -f "$out/lib/ld-linux-x86-64.so.2" ] || { echo "make_release: the loader was not bundled" >&2; exit 1; }
fi

# --system-libs: bundle the libraries a desktop Linux usually lacks; the system ones (libc, libstdc++, X11, GL, audio) stay with the system
[ "$portable" = 1 ] || mkdir -p "$out/lib"
for lib in $([ "$portable" = 1 ] || echo libraylib libturbojpeg); do
	path="$(ldd "$build/megamix" | awk -v l="$lib" '$1 ~ "^"l"\\.so" { print $3 }')"
	if [ -z "$path" ]; then
		echo "   $lib: linked statically or not found, not bundled"
		continue
	fi
	soname="$(basename "$path")"
	cp -L "$path" "$out/lib/$soname"
	echo "   bundled $soname"
done

install -m 755 "$root/tools/release/megamix.sh" "$out/megamix.sh"
install -m 644 "$root/tools/release/megamix.cfg" "$out/megamix.cfg"
sed -e "s/@VERSION@/$version/" -e "s/@SYSTEM@/Linux $arch/" -e "s/@ARCH@/$arch/" "$root/tools/release/README.txt" >"$out/README.txt"
for doc in README.md OPTIONS.md JOBS.md; do cp "$root/docs/$doc" "$out/docs/$doc"; done

echo "== check the package holds no game data"
bad="$(find "$out" -type f \( -iname '*.dol' -o -iname '*.arc' -o -iname '*.lzs' -o -iname '*.brres' -o -iname '*.brsar' \
	-o -iname '*.brstm' -o -iname '*.thp' -o -iname '*.bnr' -o -iname 'museum.bin' -o -iname 'banner.bin' \) -print)"
if [ -n "$bad" ]; then
	echo "make_release: game data in the package, refusing:" >&2
	echo "$bad" >&2
	exit 1
fi

if [ -n "$check" ]; then
	echo "== boot check against $check"
	tmp="$(mktemp -d)"
	cp -r "$out" "$tmp/pkg"
	# from another working directory, with a throw-away save folder, a relative-free --data and a frame limit
	log="$tmp/boot.log"
	if ! (cd "$tmp" && MEGAMIX_SAVE="$tmp/save" timeout 120 nice -n 19 ./pkg/megamix.sh --dump-tree 200 --no-audio --data "$check" >"$log" 2>&1); then
		echo "make_release: the packaged game did not boot (log: $log)" >&2
		tail -20 "$log" >&2
		exit 1
	fi
	grep -m1 '^data: ' "$log" | sed 's/^/   /' || true
	echo "   booted 200 frames"
	rm -rf "$tmp"
fi

echo "== archive"
tar -C "$dist" -czf "$out.tar.gz" "$name"
du -sh "$out" "$out.tar.gz" | sed 's/^/   /'
echo "done: $out.tar.gz"
