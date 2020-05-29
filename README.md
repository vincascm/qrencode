# qrencode: qrencode is a wrapper of libqrencode with libpng for lua.

qrencode is a wrapper of [libqrencode](http://fukuchi.org/works/qrencode/) with libpng for lua.

## Install

qrencode is dependent on [libqrencode](http://fukuchi.org/works/qrencode/) 
and [libpng](http://www.libpng.org/pub/png/libpng.html), so make sure these are installed
before compile it.

### Centos

```shell
yum install -y qrencode-devel libpng-devel
make
```

## Example usage

### simple usage

```lua
qr = require "qrencode"

-- print PNG data stream to stdout.

print(qr.encode("is ok?"))
print(qr:encode("is ok?"))
print(qr("is ok?"))
print(qr {text = "is ok?", level = "M"})

-- or pass a table :

print(qr {
    text="is ok?",
    level="L",
    kanji=false,
    size=4,
    margin=2,
    symversion=0,
    dpi=78,
    casesensitive=false,
    foreground="48AF6D",
    background="3FAF6F"
}
)


```

### in [nginx lua](https://github.com/openresty/lua-nginx-module)

```lua
local qr = require "qrencode"
local args = ngx.req.get_uri_args()

ngx.header.content_type = 'image/png'
ngx.say(
  qr {
    text = args.text,
    size = args.size or 8,
    margin = args.margin or 1,
    symversion = 2,
    level = 'M',
    foreground = args.fg,
    background = args.bg
  }
)
```

or nginx config file

```nginx
server {
    server_name qr.corp.com;

    location /qr {
        content_by_lua_block {
            local qr = require "qrencode"
            local args = ngx.req.get_uri_args()

            ngx.header.content_type = 'image/png'
            ngx.say( qr {
                text = args.text,
                size = args.size or 8,
                margin = args.margin or 0,
                symversion = 2,
                level = 'M',
                foreground = args.fg,
                background = args.bg
            })
        }
    }   
}
```

and visit `http://qr.corp.com/qr?text=works` to test it.

when pass a table, "text" is required and other is optional.

## Author

vinoca <https://www.vinoca.org/>

## Copyright and license

Code and documentation copyright 2014-2020 vinoca. Code released under the MIT license.
Docs released under Creative commons.
