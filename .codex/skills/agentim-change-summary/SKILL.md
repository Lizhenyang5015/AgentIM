---
name: agentim-change-summary
description: "Apply the user's personal AgentIM workflow when modifying source code, project configuration, or maintained project documentation."
---

# AgentIM personal workflow

Use this skill whenever modifying AgentIM source code, project configuration,
or maintained project documentation.

## Documentation routing

`docs/project-index.md` is the entry point for all project-wide technical rules. Before making a change, read the index and then read the topic-specific document it routes to.

- Use `docs/architecture.md` for module responsibilities and dependency direction.
- Use `docs/coding-conventions.md` for C++ language, code structure, include order, and Qt ownership rules.
- Use `docs/build-and-debug.md` for CMake, build, output, installation, and debugging workflows.
- Use `third_party/README.md` and `third_party/manifests/` for dependency layout, acquisition, versions, integrity, and runtime inventory.
- Use the relevant module README, including `src/web_cef/README.md`, for module-specific ownership and implementation status.
- Use `platform/` and `packaging/README.md` for platform runtime integration and release staging.
- Use the build, test, and packaging contracts in `docs/project-index.md` when the change affects those areas.
- Update the relevant document and the index when a maintained contract changes. Do not duplicate technical rules in this skill.

## Personal delivery workflow

When the current response creates, edits, renames, or deletes project files, include a `Files changed` section in the final response.

- List every affected source, build, debugger, formatter, script, project configuration, maintained documentation, or project Skill file from the current response exactly once.
- Use absolute clickable file links and add a short description of each change.
- Include deleted or renamed files explicitly when applicable.
- Do not list files merely read, inspected, generated in ignored build directories, or changed by external applications.
- If no project files were changed, say `No project source or configuration files were changed.`

This delivery step is part of the user's personal workflow; it does not authorize unrelated edits.
