# Design integration

## Figma MCP

AgentIM uses the Figma remote MCP server through the workspace configuration in `.vscode/mcp.json`. The configuration contains no credentials and can be shared by the team. Each developer authorizes the connection with their own Figma account in VS Code.

## Connection workflow

1. Open the Command Palette and run `MCP: List Servers`.
2. Start the `figma` server.
3. Complete the Figma authorization flow in the browser window.
4. Use an MCP-capable VS Code agent to inspect selected frames, components, variables, layout data, and exportable assets.

## Usage boundary

Use Figma as the visual specification for Qt UI work. Generated or suggested code must still follow `docs/architecture.md` and `docs/coding-conventions.md`, and must be reviewed in the running desktop application.
