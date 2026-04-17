#!/usr/bin/env bash
# .github/scripts/create-pkg.sh
# ─────────────────────────────────────────────────────────────────────────────
# Builds a macOS .pkg installer for a JUCE plugin.
# Packages VST3 and/or AU from the standard JUCE CMake artefacts layout:
#   build/<PLUGIN_NAME>_artefacts/<BUILD_TYPE>/VST3/<PLUGIN_NAME>.vst3
#   build/<PLUGIN_NAME>_artefacts/<BUILD_TYPE>/AU/<PLUGIN_NAME>.component
#
# Usage:
#   bash .github/scripts/create-pkg.sh v1.2.0
#
# Environment (all have defaults):
#   PLUGIN_NAME   default: CrushClone
#   BUILD_TYPE    default: Release
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

VERSION="${1:-dev}"
VERSION_NUM="${VERSION#v}"          # strip leading 'v'

PLUGIN_NAME="${PLUGIN_NAME:-CrushClone}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

BUILD_DIR="build"
ARTEFACTS="${BUILD_DIR}/${PLUGIN_NAME}_artefacts/${BUILD_TYPE}"
DIST_DIR="dist"

echo "▶  Packaging ${PLUGIN_NAME} ${VERSION} for macOS…"
mkdir -p "${DIST_DIR}"

# ─────────────────────────────────────────────────────────────────────────────
# Helper: build one component .pkg; appends to COMPONENT_PKGS array
# ─────────────────────────────────────────────────────────────────────────────
COMPONENT_PKGS=()

package_format() {
  local label="$1"           # human-readable: VST3 | AU
  local src="$2"             # path to the built bundle
  local install_loc="$3"     # macOS install path
  local identifier="$4"      # reverse-DNS bundle id
  local out="${DIST_DIR}/${PLUGIN_NAME}-${label}.pkg"

  if [ ! -e "${src}" ]; then
    echo "  ⚠  ${label} not found at '${src}' — skipping"
    return 0
  fi

  echo "  📦  ${label}  →  ${out}"
  pkgbuild \
    --component    "${src}" \
    --install-location "${install_loc}" \
    --identifier   "${identifier}" \
    --version      "${VERSION_NUM}" \
    "${out}"

  COMPONENT_PKGS+=("${out}")
}

# ── Build a component package per installed format ────────────────────────────

package_format "VST3" \
  "${ARTEFACTS}/VST3/${PLUGIN_NAME}.vst3" \
  "/Library/Audio/Plug-Ins/VST3" \
  "com.doxiedevops.crushclone.vst3"

package_format "AU" \
  "${ARTEFACTS}/AU/${PLUGIN_NAME}.component" \
  "/Library/Audio/Plug-Ins/Components" \
  "com.doxiedevops.crushclone.au"

# ─────────────────────────────────────────────────────────────────────────────
# Bail early if nothing was found
# ─────────────────────────────────────────────────────────────────────────────
if [ "${#COMPONENT_PKGS[@]}" -eq 0 ]; then
  echo ""
  echo "✖  No plugin formats found."
  echo "   Expected artefacts under: ${ARTEFACTS}/"
  echo "   Check your CMakeLists.txt targets (FORMATS VST3 AU) and that the"
  echo "   build succeeded before running this script."
  exit 1
fi

# ─────────────────────────────────────────────────────────────────────────────
# Synthesise a distribution.xml that lists all component packages, then
# combine them into a single installer .pkg with productbuild.
# ─────────────────────────────────────────────────────────────────────────────
DIST_XML="${DIST_DIR}/distribution.xml"
FINAL_PKG="${DIST_DIR}/${PLUGIN_NAME}-${VERSION}-macOS.pkg"

echo "  📝  Synthesising distribution.xml from ${#COMPONENT_PKGS[@]} component(s)…"

# Build the --package flags dynamically
PKG_FLAGS=()
for p in "${COMPONENT_PKGS[@]}"; do
  PKG_FLAGS+=(--package "$p")
done

productbuild --synthesize "${PKG_FLAGS[@]}" "${DIST_XML}"

echo "  🗜   Building combined installer…"
productbuild \
  --distribution "${DIST_XML}" \
  --package-path "${DIST_DIR}" \
  "${FINAL_PKG}"

# Clean up intermediate component packages (keep only the final combined .pkg)
for p in "${COMPONENT_PKGS[@]}"; do
  rm -f "${p}"
done
rm -f "${DIST_XML}"

SIZE=$(du -sh "${FINAL_PKG}" | cut -f1)
echo ""
echo "✔  Created: ${FINAL_PKG}  (${SIZE})"
