
qr = require("qrencode")

print(qr.encode("is ok?"))
print(qr:encode("is ok?"))
print(qr("is ok?"))

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
    foreground="48AF6D",
    background="3FAF6F"
}
)

print(qr {text = "is ok?", ansi = false})
