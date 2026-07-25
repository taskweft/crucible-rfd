---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: devops,dependencies
stage: mvp
---

# RFD 0020: Vendored dependencies

## Strategy

External C libraries are vendored via `git subtree` to eliminate
network dependencies from the build. The CI builds from vendored
source — no `git clone`, no `curl`, no apt for library source.

## Vendored libraries

| Library | Path | Upstream | Size |
|---------|------|----------|------|
| libh2o | `vendor/h2o/` | github.com/h2o/h2o | ~14 MB |

## Adding a new vendored dep

```bash
git subtree add --prefix vendor/<name> <upstream-url> <branch> --squash
```

Update `CMakeLists.txt` (or Makefile) to find the library in `vendor/`.

## Why not a package manager

- **apt**: versions differ between CI (Ubuntu 24.04) and Fly.io (Ubuntu 26.04)
- **brew**: macOS only, not available in CI
- **vcpkg/conan**: add complexity without benefit for 1-2 C libraries
- **git submodules**: no squash, pinned to exact commit, harder to update

`git subtree --squash` gives a single squashed commit in our tree,
easy to update via `git subtree pull --squash`.

## Why not build from upstream URL in CI

Vendoring eliminates:
- Network failures (GitHub outages, rate limits)
- Version drift (upstream branch moves)
- CI build time (no git clone of large repos)
- Dependency on external service availability

## See also

- **RFD 0017**: CI/CD pipeline (builds from vendor/)
- **RFD 0019**: Deployment (Docker builds from vendor/)
