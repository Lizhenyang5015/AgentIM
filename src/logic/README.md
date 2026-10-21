# logic

A unified application-service facade for higher layers. It further encapsulates `base`, `model`, `protocol`, `storage`, and `network`, so the Qt UI, CEF web layer, and `app` do not directly call lower-level interfaces such as databases, persistent connections, or protocols.

## Services exposed to higher layers

- `LoginService`: sign-in, sign-out, authentication state, and account switching
- `ConversationService`: conversation lists, unread counts, pinning, and synchronization
- `MessageService`: sending, retrying, recalling, read receipts, and message events
- `OrganizationService`: contacts, departments, and organizational structure
- `WorkbenchService`: workbench data and application entry points

## Lower-layer orchestration

- Read or write local data through `storage`.
- Call `network` for HTTP, persistent connections, and file transfer.
- Convert between `protocol` data and `model` entities.
- Use the threading, logging, configuration, and notification capabilities in `base`.

`logic` can retain application state, execute application workflows, and publish domain events, but it must not contain Qt widgets, CEF APIs, SQL details, or low-level network I/O implementations.
