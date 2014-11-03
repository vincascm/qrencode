package = "qrencode"
version = "1.0.2-1"
source = {
    url = "qr.tar.gz",
    file = "qr.tar.gz"
}
description = {
    summary = "qrencode is a wrapper of libqrencode with libpng for lua",
    detailed = [[
        qrencode is a wrapper of libqrencode with libpng for lua.
    ]],
    license = "MIT/X11",
    homepage = "http://fukuchi.org/works/qrencode/"
}
dependencies = {
    "lua >= 5.1"
}
build = {
    type = "builtin",
    modules = {
        qrencode = {
            sources = { "qrencode.c" },
            libraries = { "png", "qrencode" },
            libdirs = {"/usr/local/lib"}
        }
    },
}

