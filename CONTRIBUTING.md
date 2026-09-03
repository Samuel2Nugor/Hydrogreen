# Contributing to MicroHydros

> **Note:** These guidelines are a starting point and may be changed or extended through agreement within the team.

This document describes how our team collaborates in the MicroHydros repository.

## Main branch

The `main` branch should contain stable and reviewed work.

Do not commit or push changes directly to `main`.

## Branches

Create a separate branch for each task.

Branch names should use the following format:

- `feature/short-description` for new functionality
- `fix/short-description` for bug fixes
- `docs/short-description` for documentation
- `test/short-description` for tests
- `chore/short-description` for project setup and maintenance

Example:

```text
chore/repository-setup
```

## Commits

Commits should be small enough for the team to understand and review.

Commit messages should briefly describe the change.

Examples:

```text
docs: add initial project requirements
chore: create repository structure
feature: read internal temperature sensor
fix: handle failed sensor reading
```

## Pull requests

All changes must be merged into `main` through a pull request.

Each pull request should explain:

- What was changed
- Why it was changed
- How the change was checked or tested

A pull request requires approval from at least two other team members before it can be merged.

Review comments and requested changes must be handled before merging.

## Testing and checks

Relevant tests or documented checks should be completed before a pull request is merged.

Documentation-only changes should be checked for correctness and readability.

## Merging

Use a merge commit when merging a pull request.

Do not squash the commits because the project history should clearly show the contributions made by each team member.