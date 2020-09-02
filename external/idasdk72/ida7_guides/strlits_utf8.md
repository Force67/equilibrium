
### UTF-8 files: specifying a `CULTURE` for IDA to provide the best auto-analysis

In case the default database encoding is UTF-8, however, IDA cannot
derive a 'culture' from it.

In that case, IDA will consider by default that all non-ASCII
codepoints are not acceptable. That's because accepting all non-ASCII
codepoints by default, would possibly bring too many false positives.

To change that behavior, you can specify the `CULTURE`
configuration directive to match what you believe is the
language(s) that the binary file's strings are encoded in.

For example, in an UTF-8 Android Dalvik file that contains some
French text, IDA might fail to recognize the following string:
<pre style='overflow: auto'>
.rodata:00007C04     aLaMemoireAEtE  db 'La mémoire (…)
</pre>

…and turn it into double-words instead at the end of the
auto-analysis:
<pre style='overflow: auto'>
.rodata:00007C04     dword_7C04      dd 4C61206Dh, 0C3A96D6Fh, (…)
</pre>

In order to fix this, you can specify the 'culture' for IDA to
consider the acceptable set of non-ASCII codepoints for that file:
<pre style='overflow: auto'>
ida -dCULTURE=Latin_1 &lt;file&gt;
</pre>

…and IDA will be able to determine that there is indeed a string there:
<pre style='overflow: auto'>
.rodata:00007C04     aLaMemoireAEtE  db 'La mémoire (…)
</pre>

### `CULTURE=all`: accept codepoints from all cultures

Although in the previous section we mentioned that accepting all
codepoints by default in a string literal might lead to many
false positives, it is still possible to instruct IDA to do
so, by using the `all` wildcard:

<pre style='overflow: auto'>
ida -dCULTURE=all &lt;file&gt;
</pre>

