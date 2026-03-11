# Manual Release SOP (Tag + Verification)

> Use this when automation is flaky and you still need a safe, repeatable release process.

## Preconditions

- Branch: `main`
- Working tree clean
- CI green on latest `main`
- Target version not published before (SemVer)

## 1) Sync and verify local state

```bash
git checkout main
git pull --ff-only
```

Optional local validation (recommended):

```bash
source .venv/bin/activate
pio test -e native
pio run -e uno
```

## 2) Create and push release tag

```bash
git tag vX.Y.Z
git push origin vX.Y.Z
```

Example:

```bash
git tag v0.1.5
git push origin v0.1.5
```

## 3) Verify GitHub Actions execution

Expected release workflow behavior:

1. Validate (tests/build)
2. Publish to PlatformIO (only after validation success)
3. Update/create GitHub Release notes with install snippet

## 4) Verify PlatformIO registry result

```bash
pio pkg show --type library djwinter29/IOFusion
```

Registry page:

- https://registry.platformio.org/libraries/djwinter29/IOFusion

## 5) Failure handling

### Case A: Validation failed

- Fix code/tests on branch
- Merge fix to `main`
- Create a **new** tag version (`vX.Y.(Z+1)`)

### Case B: Publish failed (validation passed)

- Check `PLATFORMIO_AUTH_TOKEN`
- Confirm version does not already exist
- Re-run workflow if failure is transient
- If version already consumed, bump patch and release new tag

### Case C: Registry propagation delay

- Wait and retry `pio pkg show`
- Confirm final visibility before announcing release

## Rules

- Never force-push/retarget an already published release version.
- Never reuse version numbers.
- Prefer patch bump for recovery (`x.y.z -> x.y.(z+1)`).
