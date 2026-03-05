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

1. Keep `library.json` version as a placeholder (for example `0.0.0`).
2. Merge changes to `main`.
3. Create and push a release tag:

```bash
git checkout main
git pull

git tag v0.1.3
git push origin v0.1.3
```

4. GitHub Action runs automatically:
   - native tests
   - firmware build (`uno`)
   - set `library.json` version from tag (`v0.1.3` -> `0.1.3`)
   - PlatformIO publish

5. Verify package appears in registry:

```bash
pio pkg show --type library djwinter29/IOFusion
```

Registry page:

- https://registry.platformio.org/libraries/djwinter29/IOFusion

## 3) Safety rules

- Tag must use semantic version format: `vX.Y.Z`.
- Do not reuse an already published version.
- `library.json` is auto-updated in the workflow before publishing.
- If publish is accepted but not visible immediately, wait for registry processing delay.

## 4) Troubleshooting

### A) Missing secret

Symptom: workflow fails at login step.

Fix: add `PLATFORMIO_AUTH_TOKEN` in GitHub Actions secrets.

### B) Invalid tag format

Symptom: workflow fails when setting version from tag.

Fix: use semantic version tags such as `v0.1.3` (the workflow converts it to `0.1.3`).

### C) Duplicate version publish

Symptom: publish rejected.

Fix: bump patch version (e.g., `0.1.3` -> `0.1.4`) and push new tag.
