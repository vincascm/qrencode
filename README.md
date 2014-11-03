# qrencode: qrencode is a wrapper of libqrencode with libpng for lua.

qrencode is a wrapper of [libqrencode](http://fukuchi.org/works/qrencode/) with libpng for lua.

## Install

qrencode is dependent on [libqrencode](http://fukuchi.org/works/qrencode/) 
and [libpng](http://www.libpng.org/pub/png/libpng.html), so make sure these are installed
before compile it.

## Example usage

```lua
qr = require "qrencode"

-- print PNG data stream to stdout.

print(qr.encode("is ok?"))
print(qr:encode("is ok?"))
print(qr("is ok?"))

-- print ansi char
print(qr {text = "is ok?", ansi = true})

-- or pass a table :

print(qr {
    text="is ok?",
    level="L",
    kanji=false,
    ansi=true,
    size=4,
    margin=2,
    symversion=0,
    dpi=78,
    casesensitive=false,
    foreground="#48AF6D",
    background="#3FAF6F"
}
)


```

when pass a table, "text" is required and other is optional.

## Author

vinoca <http://www.vinoca.org/>

## Copyright and license

Code and documentation copyright 2014-2015 vinoca. Code released under the MIT license.
Docs released under Creative commons.
