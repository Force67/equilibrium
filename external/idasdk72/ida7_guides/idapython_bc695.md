
# IDA 7.0: IDAPython backward-compatibility with 6.95 APIs

Table of contents:

1. <a href="#intended">Intended audience</a>
1. <a href="#reasons">Reasons for API changes in 7.0</a>
1. <a href="#how">How backward-compatibility APIs work</a>
1. <a href="#availability">Availability of the 6.95 backward-compatibility APIs</a>
1. <a href="#take-action">What should I do if...?</a>
1. <a href="#coverage">Coverage of the backward-compatibility code</a>


## <span id="intended">Intended audience</span>

IDAPython script/processor module writers should have a
look *at least* at the following sections:

* <a href="#availability">Availability of those APIs</a>
* <a href="#take-action">Porting exsting, or writing new scripts</a>

## <span id="reasons">Reasons for API changes in 7.0</span>

IDA 7.0 consists of `x86_64` binaries (as opposed to all previous versions,
which consisted of `i386` binaries). This has the very unfortunate
side-effect that all existing binary plugins will stop working.

Thus, since ABI compatibility is gone and therefore those binary plugins
would require recompiling, we decided that now would be a perfect time
to perform a much-needed API cleanup:

- renaming inconsistently-named functions
- renaming inconsistently-named constants
- removing deprecated functions
- improving some structures & classes

Binary plugin authors will not only have to recompile: first, a small
porting effort will be needed in order to adapt to the new, cleaned-up API.

---

The situation, however, is very different for IDAPython script/plugin
authors: the exact architecture of the platform where IDA runs (i.e.,
`x86_64` or `i386`) is (mostly, at least) irrelevent to them.

Consequently, we decided we would provide a compatibility layer, that
maps the 'old' 6.95 APIs, to the new 7.0 ones, in order to ease the
adoption of IDA 7.0 in as many cases as possible.

## <span id="how">How backward-compatibility APIs work</span>

Backward-compatibility is provided by python.cfg's
`AUTOIMPORT_COMPAT_IDA695` directive.

When that directive is set to `YES`, additional code will be loaded
into IDAPython, providing mappings for the older function names, classes,
constants, etc...

With that directive turned on, existing scripts should just work.
If that isn't the case, please contact us on
[support@hex-rays.com](mailto:support@hex-rays.com) and we'll try our
best and fix IDAPython so that it covers your use-case.

This is not, however, a viable & long-term solution:

- the amount of code & general overhead that loading these APIs adds, is not quite negligible.
- it 'pollutes' to the `ida_*` (and `idaapi`) modules.
- (admittedly to a lesser degree) it slows down development & evolution of IDAPython.

## <span id="availability">Availability of the 6.95 backward-compatibility APIs</span>

### At 7.0 release-time

The `AUTOIMPORT_COMPAT_IDA695` directive is turned on by default, which
means that existing scripts should work.

### In a later release (7.1, or 7.2)

When some time has passed, a later release of IDA will ship with
`AUTOIMPORT_COMPAT_IDA695` set to `NO` by default.
Of course, users can still turn it back on, but that will at
least hint the user that something might require attention.

### In the future

After some time (it's difficult to provide a time frame, here. We'll
have to see how things go), we'll simply remove the
backward-compatibility code. It will then be impossible for
scripts that were not ported to function.

## <span id="take-action">What should I do if...?</span>

### You have existing scripts/plugins

If your script(s) is(are) meant to work with IDA 7.0 onwards,
it might be a good idea to port them as soon as possible.

The modifications should be (in almost all cases) trivial,
since many API changes consist of function, types & constants
renaming.

Please have a look at [the 6.95 &rarr; 7.0 API guide](api_map.html)
for information about that renaming.

A very good test to test whether your scripts have been
properly ported, is to set `AUTOIMPORT_COMPAT_IDA695` to
`NO`,restart IDA, and try your scripts again.

### You are developing new scripts/plugins

The best course of action here is to simply set to
`AUTOIMPORT_COMPAT_IDA695` to `NO` from the beginning, and
write your script/plugin directly with the new API.

## <span id="coverage">Coverage of the backward-compatibility code</span>

We did what was reasonably feasible, to provide an IDAPython API
that's as backward-compatible as possible with the IDA 6.95 API

However, we considered it unreasonable for some parts of the API
to be ported. Most notably:

1. the "processor module" API: existing processor modules will
   have to be ported to the new API. Please see the SDK's
   `module/script/proctemplate.py` (or any other `*.py` file in
   that directory) for examples how to use the new API.
1. processor module-related notifications: some of those have
   either been renamed, or have possibly changed signature

Most (all?) of the renamed functions, constants, etc... should be
covered, in all modules: `ida_*`, `idaapi`, `idc`, ...

If something doesn't work/isn't there anymore, it's likely an
omission from our side. In that case, please let us know about
any missing bits & pieces, that you believe should be there and
that we might have forgotten!
