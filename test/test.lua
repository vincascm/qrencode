
qr = require("qrencode")

print(qr.encode("is ok?"))
print(qr:encode("is ok?"))
print(qr("is ok?"))

print(qr {text = "is ok?", ansi = true})
print(qr {
    text="123",
    level="L",
    kanji=false,
    ansi=false,
    size=3,
    symversion=0,
    dpi=80,
    casesensitive=true,
    eightbit=false,
    foreground="#48AF6D",
    background="#3FAF6F"
}
)

