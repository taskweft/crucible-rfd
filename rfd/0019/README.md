---
authors: K. S. Ernest (iFire) Lee <fire@users.noreply.github.com>
state: ideation
discussion:
labels: devops,deployment
stage: mvp
---

# RFD 0019: Deployment — Docker + Fly.io

## Docker

Multi-stage build in `Dockerfile`:

| Stage | Base | Contents |
|-------|------|----------|
| `compile` | `buildpack-deps:26.04` | Builds h2o from vendor/, installs FDB client, builds crucible-demo via cmake |
| `runtime` | `ubuntu:26.04` | FDB server + client, crucible-demo binary, wrk, entrypoint |

The entrypoint (`entrypoint.sh`):
1. Start FoundationDB via `fdbmonitor`
2. Wait for FDB readiness (poll `fdbcli status minimal` up to 30s)
3. Start crucible-demo with `$WORKERS` worker threads
4. Run wrk benchmark against `/zone?n=200`
5. Print results

## Fly.io

`fly.toml` deploys to the `yul` region:

| Setting | Value | Cost |
|---------|-------|------|
| CPU | shared-cpu-2x (2 cores) | ~$7/mo |
| RAM | 1 GB | included |
| Region | yul (Montreal) | — |
| Workers | 2 | matches 2 cores |

Gall's Law: deploy the smallest working configuration first.
Shared-cpu-2x demonstrates linear scaling (1 worker vs 2 workers
on 2 cores). Scale up to dedicated-cpu when the MUD POC proves
the architecture.

## Local testing

```bash
docker build -t crucible-demo .
docker run --rm -it -p 8080:8080 crucible-demo
# or via Podman Quadlet on macOS:
# podman machine ssh → systemctl --user start crucible-demo
```

## See also

- **RFD 0017**: CI/CD pipeline (builds the Docker image)
- **RFD 0005**: Simulation model — 64 Hz ticked
- `Dockerfile`, `fly.toml`, `entrypoint.sh`
