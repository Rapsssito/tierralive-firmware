# Project Aqua ESPHome

ESPHome configuration and tooling for the Project Aqua IoT device.

## Folder structure

```
.github/workflows/   # CI + release automation
config/              # ESPHome YAML configs
  secrets.yaml       # (local, gitignored)
  secrets.yaml.example
esphome.version       # ESPHome image tag
scripts/             # helper scripts (docker-based)
```

## Quick start

1. Copy secrets template:
   - `cp config/secrets.yaml.example config/secrets.yaml`
2. Run ESPHome:
   - `scripts/run.sh livingroom.yaml`

## Release process

This repo uses Conventional Commits and semantic-release.
- Commit format: `feat:`, `fix:`, `chore:`, etc.
- Releases are generated automatically on pushes to `main`.
