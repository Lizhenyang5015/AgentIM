# Source modules are arranged by responsibility. They contribute sources to
# the single desktop application target rather than producing standalone libs.
# The dependency direction is:
# base -> model/protocol -> storage/network -> logic -> ui_qt/web_cef -> app.
add_subdirectory(web_cef)
add_subdirectory(app)
add_subdirectory(base)
add_subdirectory(ui_qt)
