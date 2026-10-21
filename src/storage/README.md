# storage

Local persistence layer: SQLite database wrappers, migrations, caches, file indexes, and storage for User, Conversation, and Message data.

All database access must go through this module; the UI and application layers must not operate SQLite directly.
