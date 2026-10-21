# Coding conventions

## C++ language baseline

- Use C++20. Compiler extensions are disabled.
- Prefer clear ownership, narrow interfaces, and explicit dependencies.
- Keep module implementation details private and expose only the interfaces required by higher layers.

## Include order

Order `#include` directives from lower-level dependencies to higher-level project layers. Use blank lines between groups and sort headers alphabetically within each group.

1. C++ standard-library headers.
2. Third-party and system/framework headers, including Qt.
3. AgentIM `base` headers.
4. AgentIM business-layer headers: `model`, `protocol`, `network`, `storage`, and `logic`.
5. AgentIM presentation and entry-layer headers: `platform`, `ui_qt`, and `app`.

For implementation files, place the implementation's own header in the group that matches its layer. Do not force it to be the first include when doing so violates the dependency order.

## Qt Widgets ownership

Make the lifetime and ownership of every UI object explicit.

- A `QObject` child created with a Qt parent is owned by that parent. A raw pointer member to that child is an observer and access handle, not a second owner.
- Do not combine Qt parent ownership with `std::unique_ptr` or another owning smart pointer for the same object.
- APIs such as `QMainWindow::setCentralWidget()` take ownership. Pass a heap-allocated widget and do not also manage it as a value member.
- Store every persistent `QWidget` created by a UI class as a named private pointer member of its nearest owning UI class, even when the current implementation does not manipulate it after construction.
- Keep temporary dialogs and short-lived helper objects local.
- Prefer a stable central container and `QStackedWidget` for page switching. Avoid repeatedly replacing the central widget while retaining stale raw pointers.
- For objects that another owner may delete before this class, use `QPointer<T>` when null-on-destruction observation is required.

## UI design tokens

- Shared visual values such as colors and typography must use semantic tokens
  from `agentim/ui/theme/design_tokens.h`.
- Name tokens by visual role, such as surface, border, or text purpose. Do not name them after a hexadecimal value or a single screen.
- Do not place literal hexadecimal colors in widget style sheets. Window-specific geometry and spacing may remain local when they are not shared design decisions.
- Product and design reviews should refer to the semantic token name. Changing a shared color must require editing the token definition rather than searching every widget implementation.

## UI typography

- Use `theme::typography::kUiFontFamily` for the cross-platform UI font
  fallback chain.
- Choose font sizes from the maintained type scale: micro, caption, body, and
  title. Do not add literal `px` font sizes to widget style sheets.
- Choose font weights from the maintained regular, medium, semibold, and bold
  tokens. Do not use arbitrary numeric font weights in individual widgets.
- Use body/medium for standard interactive controls, semibold for selected or
  emphasized controls, caption for supporting labels, and micro only for small
  badges.
- Add a new typography token only when the value represents a reusable visual
  role across multiple controls or screens.

## UI localization

- User-facing text must be requested through the stable IDs in `agentim/ui/i18n/localized_strings.h`. Do not place Chinese, English, or another natural-language literal directly in a widget implementation.
- Keep language catalogs centralized in the localization layer. English is the fallback language, and the current implementation selects Simplified Chinese when the operating-system language is Chinese.
- Keep brand names and language-independent symbols separate from translated text. For example, compose an icon or `+` marker with a translated label instead of adding that marker to every language entry.
- UI code must depend on semantic text IDs rather than a catalog format. A future migration to Qt `.qm` files or another localization framework should replace the localization implementation without changing window code.
