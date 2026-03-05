# Release Automation Plan (GitHub Tag -> PlatformIO Publish)

This project supports automated PlatformIO publishing through GitHub Actions when a semantic version tag is pushed.

Workflow file:

- `.github/workflows/release-publish.yml`

## 1) One-time setup

1. Open GitHub repository settings.
2. Go to **Settings -> Secrets and variables -> Actions**.
3. Add secret:
   - `PLATFORMIO_AUTH_TOKEN`
4. Ensure default branch (`main`) CI is green.

## 2) Per-release flow

1. Update `library.json` version (e.g., `0.1.3`).
2. Merge changes to `main`.
3. Create and push matching tag (must match `library.json`):

```bash
git checkout main
git pull

git tag v0.1.3
git push origin v0.1.3
```

4. GitHub Action runs automatically:
   - native tests
   - firmware build (`uno`)
   - tag/version consistency check
   - PlatformIO publish

5. Verify package appears in registry:

```bash
pio pkg show --type library djwinter29/IOFusion
```

Registry page:

- https://registry.platformio.org/libraries/djwinter29/IOFusion

## 3) Safety rules

- Tag must match `library.json.version` exactly (`vX.Y.Z` <-> `X.Y.Z`).
- Do not reuse an already published version.
- If publish is accepted but not visible immediately, wait for registry processing delay.

## 4) Troubleshooting

### A) Missing secret

Symptom: workflow fails at login step.

Fix: add `PLATFORMIO_AUTH_TOKEN` in GitHub Actions secrets.

### B) Version mismatch

Symptom: workflow fails at tag/version validation.

Fix: set `library.json.version` to match tag version and re-tag with next version.

### C) Duplicate version publish

Symptom: publish rejected.

Fix: bump patch version (e.g., `0.1.3` -> `0.1.4`) and push new tag.
