
### Non-UTF-8 files: deriving the 'culture' from the default 1-byte-per-unit encoding

By default, IDA doesn't have a `CULTURE` specified in its
`ida.cfg` file. Instead it will try to derive the 'current culture'
from the default 1-byte-per-unit encoding (provided that encoding
is not UTF-8)

Whether that encoding is specified using the `ENCODING` directive, or
if it is guessed from the system's locale, IDA will derive the 'current
culture' from that encoding using the following table in `ida.cfg`:
<pre>
ENCODING_CULTURES =
        1250: Central_Europe,
        1251: Cyrillic,
        1252: Latin_1,
        1253: Greek,
        1254: Turkish,
        1255: Hebrew,
        1256: Arabic,
        1257: Baltic,
        (…)
</pre>

For example, if the default 1-byte-per-unit encoding is `CP1252`,
IDA derived that the 'culture' is `Latin_1`, causing auto-analysis
to discover the following string in a file:
<pre>
.data:00042CA4     aLeCoteDroit    DCB "Le côté (…)
</pre>

…but if that encoding is something else (e.g., `CP1251`),
then you might end up with this instead:
<pre>
.data:00042CA4     dword_42CA4     DCD 0x6320654C, (…)
</pre>

That is because IDA derived the 'culture' from the encoding, which in
this case led to the 'Cyrillic' culture, which doesn't contain the French
letter `'é'`, causing string recognition to fail.

In order to fix this, you can run IDA like so:
<pre>
ida -dENCODING=CP1252 &lt;file&gt;
</pre>

Then, all is fine again: IDA could find that string literal:
<pre>
.data:00042CA4     aLeCoteDroit    DCB "Le côté (…)
</pre>

In addition, if you are very often disassembling files that
require that you specify a given `ENCODING`, you can simplify your
workflow by either

1. setting `ENCODING` in `ida.cfg`: `ENCODING=CP1252`
1. adding `Latin_1` as culture in `StrlitChars`:
<pre>
StrlitChars =
        (…)
        Culture_Latin_1,
        CURRENT_CULTURE;
</pre>
