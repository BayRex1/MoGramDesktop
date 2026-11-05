import pathlib
import re
import sys

target = pathlib.Path(sys.argv[1])
source = target.read_text(encoding="utf-8")
pattern = re.compile(r"(?m)^export PKG_CONFIG_PATH=.*$")
replacement = r'''DAV1D_PKG_CONFIG_DIR="$(cygpath -u "$FullScriptPath/../local/lib/pkgconfig")"
export PKG_CONFIG_PATH="$DAV1D_PKG_CONFIG_DIR:$PKG_CONFIG_PATH"

# Strawberry pkg-config 0.26 may receive FFmpeg version predicates as
# separate shell arguments. Join the module, operator and version first.
REAL_PKG_CONFIG="$(command -v pkg-config)"
if [ -z "$REAL_PKG_CONFIG" ]; then
    echo "ERROR: pkg-config was not found in PATH"
    exit 1
fi
export REAL_PKG_CONFIG
PKG_CONFIG_WRAPPER_DIR="$(mktemp -d)"
cat > "$PKG_CONFIG_WRAPPER_DIR/pkg-config" <<'EOF'
#!/usr/bin/env bash
args=()
while (($#)); do
    case "$1" in
        ">="|"<="|">"|"<"|"=")
            if (("${#args[@]}")) && (($# >= 2)); then
                last=$(("${#args[@]}" - 1))
                module="${args[$last]} $1 $2"
                unset 'args[last]'
                args+=("$module")
                shift 2
            else
                args+=("$1")
                shift
            fi
            ;;
        *)
            args+=("$1")
            shift
            ;;
    esac
done
exec "$REAL_PKG_CONFIG" "${args[@]}"
EOF
chmod +x "$PKG_CONFIG_WRAPPER_DIR/pkg-config"
export PATH="$PKG_CONFIG_WRAPPER_DIR:$PATH"
echo "FFmpeg pkg-config: $REAL_PKG_CONFIG"
echo "FFmpeg pkg-config path: $PKG_CONFIG_PATH"
pkg-config --exists --print-errors "dav1d >= 0.5.0"
'''
source, count = pattern.subn(lambda _: replacement, source, count=1)
if count != 1:
    raise SystemExit(
        "Expected exactly one PKG_CONFIG_PATH export in build_ffmpeg_win.sh; "
        f"found {count}. No changes made."
    )
target.write_text(source, encoding="utf-8", newline="\n")
print(f"Patched {target}")
