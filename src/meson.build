srcs = files(
    'wireplumber.c',
    'endpoint.c',
    'device.c',
    'video.c',
    'profile.c',
    'audio.c',
)

deps = [
    dependency('gobject-2.0'),
    dependency('gio-2.0'),
    dependency('wireplumber-0.5'),
    # dependency('json-glib-1.0'),
]

astal_wireplumber_lib = library(
    'astal-wireplumber',
    sources : srcs,
    include_directories : astal_wireplumber_inc,
    dependencies : deps,
    version : meson.project_version(),
    install : true
)

libastal_wireplumber = declare_dependency(
    link_with : astal_wireplumber_lib,
    include_directories : astal_wireplumber_inc)

# astal_wireplumber_executable = executable(
#     'astal-wireplumber',
#     files('astal-wireplumber.c'),
#     dependencies : [
#         dependency('gobject-2.0'),
#         dependency('gio-2.0'),
#         dependency('json-glib-1.0'),
#         libastal_wireplumber
#     ],
#     install : true)

pkg_config_name = 'astal-wireplumber-' + lib_so_version

if get_option('introspection')
    gir = gnome.generate_gir(
        astal_wireplumber_lib,
        sources : srcs + astal_wireplumber_headers + astal_wireplumber_subheaders,
        nsversion : '0.1',
        namespace : 'AstalWp',
        symbol_prefix : 'astal_wp',
        identifier_prefix : 'AstalWp',
        includes : ['GObject-2.0', 'Gio-2.0'],
        header : 'astal-wp.h',
        export_packages : pkg_config_name,
        install : true
    )

    if get_option('vapi')
        gnome.generate_vapi(
            pkg_config_name,
            sources : [gir[0]],
            packages : ['gobject-2.0', 'gio-2.0'],
            install : true)
    endif
endif

pkg_config.generate(
    name : 'astal-wireplumber',
    version : meson.project_version(),
    libraries : [astal_wireplumber_lib],
    filebase : pkg_config_name,
    subdirs : 'astal',
    description : 'astal wireplumber module',
    url : 'https://github.com/astal-sh/wireplumber')
