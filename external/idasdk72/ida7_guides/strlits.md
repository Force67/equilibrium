# IDA 7.0: Automatic discovery of string literals during auto-analysis

## Intended audience

Experienced, power users wishing to obtain the best initial auto-analysis
results, in particular on files containing non-ASCII string literals.

Note that IDA usually already provides very good results out of the box,
so the information below is really for expert, fine-tuning purposes.

## What is this about?

When it performs its initial auto-analysis IDA will, among many other things,
look for string literals in the segments that were loaded from the file.

That "looking for string literals" relies rather heavily on heuristics, to
tell possible string literal from other things. _Some of_ the concepts
used by those heuristics are:

1. length of candidate string
1. proximity of other strings
1. whether characters of candidate strings are printable
1. whether characters are part of `ida.cfg`'s set of acceptable chars
   in a string literal
1. whether characters met in the candidate string are either ASCII,
   or for those that are non-ASCII if they are all part of the same
   language
1. …

The rest of this document will focus on the 4th item:
the set of acceptable chars in a string literal.

## Preamble: a word about string literals encodings in IDA 7.0

Prior to IDA 7.0, string literals were just treated as strings of bytes,
and it was assumed that the locale's encoding should be used whenever
decoding those into actual, displayable strings.

That worked satisfyingly well, but led to many false positives, and
the impossibility to have IDA perform the best auto-analysis possible,
even when the user knew what specific encodings were used in the file.

IDA 7.0 changes that, and _always_ assigns default encodings for
encodings with 1-, 2- and 4-bytes-per-unit.

* Example 1-byte-per-unit encodings are: CP1252, CP1251, UTF-8
* Example 2-bytes-per-unit encodings are: UTF-16
* Example 4-bytes-per-unit encodings are: UTF-32

Unless one is specified, IDA will 'guess' those encodings, and for
the 1-byte-per-unit encoding, it'll do so in the following manner:

* if the file is a typical Windows or DOS binary (i.e., `PE`, `EXE` or `COM`), then
  * if running on Windows, then use the locale codepage
  * else (i.e., running on Linux or OSX) default to `CP1252`
* otherwise,
  * default to UTF-8

Those are the "best guess" defaults and they are, in effect, not very
different from what was happening in IDA before version 7.0

### Overriding the default 1-byte string encoding: the `ENCODING` configuration directive

Specifying `ENCODING` in the `ida.cfg` configuration file (or
on the command line) lets the user inform IDA that the bytes in a
1-byte-per-unit string literal, are encoded using that encoding.

Now that the default (or `ENCODING`-specified) encoding topic is
covered, let's get back to the root of the problem..

## The problem

Before 7.0, IDA would use `ida.cfg`'s (somewhat confusingly-named)
`AsciiStringChars` directive, to determine what bytes were possibly
part of a string literal.

That `AsciiStringChars` directive is a byte string, which contains
essentially all printable ASCII chars as well as a subset of the
upper 128 values of the `[0-256)` range.

The most visible problems with this are:

* whenever a user wants to improve `AsciiStringChars` to match the set of
  bytes that look valid in a different encoding, the user typically has to:
    * look up that encoding definition, to see what values above 0x7F are
      likely valid string literal characters in that encoding
    * encode those in the global `ida.cfg` file, which can be pretty tricky
      if the user's editor is not setup to work in that target encoding:
      it will show those byte values as other characters
* no support for UTF-8 sequences: `AsciiStringChars` doesn't support multibyte
  encodings. If the user is analyzing, say, a Linux binary file, it's
  likely that non-ASCII string literals are encoded using a multibyte
  encoding such as UTF-8. There was no way for the user to express what
  non-ASCII UTF-8 sequences are acceptable, in `ida.cfg`.

## The solution

Instead of `AsciiStringChars` consisting of a C-like string of bytes
describing the acceptable set of characters, we have:

- renamed `AsciiStringChars` to the less ambiguous `StrlitChars`
- bumped `StrlitChars` into something more evolved, which can
  contain not only character literals, but also different
  forms of content

Let's look at those..

## `StrlitChars` format

The new `StrlitChars` is composed of a sequence of entries. E.g.,

<pre>
StrlitChars =
        "\r\n\a\v\b\t\x1B"
        " !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~",
        u00A9, // COPYRIGHT SIGN
        u00AE, // REGISTERED SIGN
        u20AC, // EURO SIGN
        u00B0, // DEGREE SIGN
        u2013, // EN DASH
        u2014, // EM DASH
        Culture_Latin_1,
        CURRENT_CULTURE;
</pre>

We can observe that:

* entries are separated by `','` (commas)
* string literals are accepted, which allows adding ASCII printable characters very easily
* Unicode codepoints (`uXXXX` entries) are accepted
* you can add a whole 'culture' to the set of accepted characters/codepoints
* you can add the 'current culture' to the set of accepted characters/codepoints

When IDA starts, it will compile that directive into an efficient lookup
table, containing all the codepoints that were specified, and that lookup
table will be used just like `AsciiStringChars` was used to determine
what codepoints are acceptable in a string literal.

Let's now take a closer look at the notions of 'culture' and
'current culture'.

## What's a "culture"

First of all, let's be blunt: we use the term 'culture'
for lack of a better word. It doesn't represent an actual
culture in terms of history, tradition, …

A 'culture' in IDA is a quick way to represent a set of codepoints,
that conceptually belong together. Typically, those 'culture's will
contain many letters, but very few symbol or punctuation codepoints
(in order to reduce the number of false positives in automatic string
detection.)

As an example, if we wanted to add the set of characters supported by
the "Western Europe" charsets to the `StrlitChars` directive
_without using 'cultures'_, we could do it like so:

<pre>
StrlitChars =
        "\r\n\a\v\b\t\x1B"
        " !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~",
        u00A9, // COPYRIGHT SIGN
        u00AE, // REGISTERED SIGN
        u20AC, // EURO SIGN
        u00B0, // DEGREE SIGN
        u2013, // EN DASH
        u2014, // EM DASH

        // latin1 culture start
        u00C0..u00D6,
        u00D8..u00F6,
        u00F8..u00FF
        u0192,
        u0160,
        u0152,
        u017D,
        u0161,
        u0153,
        u017E,
        u0178,
        -u00FF; // remove LATIN SMALL LETTER Y WITH DIAERESIS (prevents many false positives in automatic string literals)
        // latin1 culture end
        CURRENT_CULTURE;
</pre>

Note that we just introduced two additional syntactic possibilities [\[1\]](#ucdr_format), here:

1. Unicode codepoint range: `uXXXX..uXXXX` (end inclusive)
1. Codepoint suppression: `-uXXXX`

As you can guess, it can become a tad tedious -- and Latin 1 is
simple, but if I wanted to add the characters that
are likely to be found in, say, the "Baltic" culture (which roughly
corresponds to codepage `CP1257`), I would have had to
add ~70 disjoint codepoints, which makes it become cryptic & error-prone.

### How do I know what 'cultures' are available

IDA ships with a predefined set of 'culture' files.
They can be found in the `cfg/` directory:
<pre>
path/to/ida/install/cfg/$ ls -lh *.clt
… Baltic.clt
… Central_Europe.clt
… Greek.clt
… Japanese.clt
… Latin_1.clt
… Turkish.clt
… Vietnam.clt
path/to/ida/install/cfg/$
</pre>

…but you are of course free to add your own, and/or modify or improve
the existing ones as needed (you can even send those back to us; they'll
be very much welcome!)

Ok, so now you know a bit about what is a 'culture' in IDA's parlance.
There's one more thing to cover though, and it's non-trivial:
the `CURRENT_CULTURE` token.

## What's the `CURRENT_CULTURE` about?

The `StrlitChars` directive will typically contain the `CURRENT_CULTURE`
directive. That instructs IDA that all codepoints derived from the
'current culture' that IDA is operating with, should be considered
valid codepoints in string literals.

### How do I know what 'current culture' IDA is using?

There can be 2 sources of information for IDA to know what 'current
culture' it should be operating with:

1. the `CULTURE` config directive (in `ida.cfg`), or
1. the default 1-byte-per-unit character encoding of the IDB,
   _if that encoding is not UTF-8_ [\[2\]](#utf8_all)
   (regardless of whether IDA assigned that default
   1-byte-per-unit character encoding, or whether the
   `ENCODING` directive was provided.)

Let's have a look at those.

### The `CULTURE` config directive

It is possible to tell IDA, at start-time, what 'culture' it should
be operating with, by setting the `CULTURE` configuration directive
in the `ida.cfg` file. E.g.,
<pre>
CULTURE="Cyrillic";
</pre>

The above statement means that IDA will load the `cfg/Cyrillic.clt` file,
parse its set of codepoints, and add that to the ones already specified
by the `StrlitChars` directive.

Therefore, when performing its initial auto-analysis, IDA will consider
valid for a string literal all codepoints defined by `StrlitChars`, and
that means:

* codepoints within the specified ASCII subset,
* or among the set of carefully-selected symbols ('COPYRIGHT_SIGN', etc..),
* or among the set the codepoints featured in the `cfg/Cyrillic.clt` file.

If you didn't specify the `CULTURE` config directive though (which
is the default), IDA will try to 'guess' the culture, from
the current 1-byte-per-unit encoding of the database, but only if that
encoding is not a multibyte encoding (e.g., UTF-8.)

However, if the encoding is UTF-8, things will be different…

<table>
<tbody>
<tr>
<td valign="top" style="background-color: #ffeeff">
<!--BODY_NONUTF8-->
</td>
<td valign="top" style="background-color: #ffffee">
<!--BODY_UTF8-->
</td>
</tr>
</tbody>
</table>

### `CURRENT_CULTURE`: wrapping up

Therefore, the user can either:

* specify an `ENCODING` for 1-byte-per-unit string literals, and if that
  encoding is not UTF-8 let IDA derive the 'current culture' from it, or
* specify a `CULTURE`, to override whatever IDA might have derived from
  the effective database 1-byte-per-unit encoding (regardless of whether
  it was guessed, or specified with `ENCODING`)

## Summary

There's a lot of non-trivial information for you to process in this
document, and by now you might be either a bit overwhelmed, or just
plain confused.

Let me sum up the information in the following manner:

On encodings:

* IDA now automatically guesses & assigns 1-byte-per-unit, 2-bpu
  and 4-bpu encodings to a database
  * That guess can be overriden by specifying an `ENCODING`
* Regardless of whether it was guessed or specified, that encoding
  can be used to derive a 'current culture'. That doesn't work for
  UTF-8 though, as that encoding covers the whole Unicode range

On `StrlitChars`:

* IDA 7.0 introduces the notion of 'culture'. A 'culture' file describes
  a set of codepoints that are conceptually grouped together, although
  they can be disjoint in the Unicode specification
* IDA 7.0 extends the previous `AsciiStringChars` directive, by making it
  capable to express much more than just 1-byte characters, and renamed
  it to `StrlitChars`
* `StrlitChars` has a rather flexible syntax, allowing for literals,
  codepoints, codepoint ranges, codepoint blocks, codepoint suppressions,
  embedding 'cultures', and even embedding the 'current culture'
* The 'current culture' is either guessed from the 1-byte-per-unit
  default encoding, or can be specified with the `CULTURE` directive
* Just as with IDA 6.95's `AsciiStringChars`, the new `StrlitChars` will
  be used by the initial auto-analysis, in order to guess possible
  string literals in the program


# Footnotes

1. <a name="ucdr_format">See `ida.cfg` for a wider coverage of the syntax</a>
1. <a name="utf8_all">UTF-8 covers the whole Unicode codepoint range, and thus a 'culture' derived from the UTF-8 encoding would be overly inclusive and turn up many false positives</a>
