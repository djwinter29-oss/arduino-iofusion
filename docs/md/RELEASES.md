# Releases

This document covers both the normal tag-driven release flow and the manual fallback procedure.

## Release Model

IOFusion uses semantic version tags to drive release automation.

- Tag format: `vX.Y.Z`
- Source repository owner: `djwinter29-oss`
- PlatformIO registry owner: `djwinter29`
- Package name: `IOFusion`
- Workflow: `.github/workflows/release-publish.yml`

## One-Time Setup

1. Open GitHub repository settings.
2. Go to `Settings -> Secrets and variables -> Actions`.
3. Add the `PLATFORMIO_AUTH_TOKEN` secret.
4. Keep `main` healthy before tagging releases.

## Normal Release Flow

### Before tagging

- Sync `main` and confirm CI is green.
- Keep `library.json` on a placeholder version such as `0.0.0`.

### Create and push the release tag

```bash
git checkout main
git pull --ff-only

git tag v0.1.6
git push origin v0.1.6
```

## What The Workflow Does

After a tag push, GitHub Actions will:

1. Validate the tag format.
2. Run native tests.
3. Build the Uno firmware.
4. Rewrite `library.json` version from the pushed tag.
5. Publish the PlatformIO package.
6. Create or update the GitHub Release with registry and install details.

## Verification

Check the published package:

```bash
pio pkg show --type library djwinter29/IOFusion
```

Registry page:

- https://registry.platformio.org/libraries/djwinter29/IOFusion

Expected install snippet:

```ini
lib_deps =
  djwinter29/IOFusion@^0.1.6
```

## Optional Local Validation Before Tagging

```bash
source .venv/bin/activate
pio test -e native
pio run -e uno
```

## Manual Fallback Procedure

Use this when the automation is flaky but the release itself is still ready.

1. Fix the release issue on `main` if needed.
2. Run the local validation commands above.
3. Push a new semantic version tag.
4. Verify whether the action recovers on re-run.
5. If a version was already consumed or partially published, bump the patch version and release again.

## Failure Handling

### Validation failed

- Fix code or tests on `main`.
- Create a new version tag such as `vX.Y.(Z+1)`.

### Publish failed after validation passed

- Confirm `PLATFORMIO_AUTH_TOKEN` exists.
- Confirm the version has not already been published.
- Re-run the workflow if the failure is transient.
- If the version is already consumed, bump the patch version and tag again.

### Registry propagation delay

- Wait a short time and retry `pio pkg show`.
- Confirm visibility before announcing the release.

## Rules

- Never reuse version numbers.
- Never force-push or retarget an already published release version.
- Prefer a patch bump for recovery.