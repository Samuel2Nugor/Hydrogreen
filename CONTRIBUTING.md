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

---

## Git workflow: every task, step by step

The rules above say *what* to do. This section is the exact command sequence for *how*. Follow it every time and conflicting pushes mostly disappear.

**1. Start from an up-to-date `main`, then branch off it.**

```bash
git checkout main
git pull
git checkout -b feature/your-task
```

Branching off an old `main` is the number-one cause of merge conflicts. Always `pull` first.

**2. Do the work. Commit in small steps with the correct prefix.**

```bash
git add <files>          # stage only the files this commit is about
git commit -m "feature: read internal temperature sensor"
```

**3. Push your branch early and often.**

```bash
git push -u origin feature/your-task     # first push
git push                                 # every push after that
```

Pushing often means your work is backed up and visible, and reduces how far your branch drifts from everyone else's.

**4. Open a pull request into `main`.** Fill in what / why / how tested. Wait for two approvals. Merge with a merge commit (never squash).

## Keeping your branch up to date

While you work, other people's PRs get merged into `main`. Before you open your PR (and any time `main` has moved), pull those changes into your branch so review is clean and the final merge is trivial:

```bash
git checkout main
git pull
git checkout feature/your-task
git merge main
```

Resolve any conflicts locally (see below), commit, and push. Do this *on your branch*, not on `main`.

## Handling conflicts

A conflict is normal and fixable. It just means two branches changed the same lines. When git reports a conflict:

1. **Do not panic, do not delete your branch, and never `git push --force`.** Force-pushing can erase a teammate's work.
2. Open the conflicted file(s). Git marks the clashing sections like this:
   ```text
   <<<<<<< HEAD
   your version
   =======
   the incoming version
   >>>>>>> main
   ```
3. Edit the file to the correct final result and delete the `<<<<<<<`, `=======`, `>>>>>>>` marker lines.
4. Stage and commit:
   ```bash
   git add <file>
   git commit
   ```
5. Push your branch.

If you are unsure what the correct result is, **ask in Discord before committing** and screenshot the conflict. Guessing and force-pushing is how history gets broken.

## Habits that prevent conflicts

- **One task = one issue = one branch = one PR.** Never mix unrelated work in one branch.
- **Never work directly on `main`.**
- **Never force-push** to a shared branch.
- **Keep branches short-lived.** Open a PR within a day or two. The longer a branch lives, the harder it merges.
- **Split work by file or folder.** If two people must touch the same file, agree who goes first in Discord.
- **Say what you are starting in Discord** before you start, so no one duplicates it.
