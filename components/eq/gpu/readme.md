# GPU Component

Abstraction on modern graphics apis (dx12 and vulkan).
This component does not aim to provide a complete renderer implementation,
but rather a higher level set of common rendering directives.

## Graphics lingo
- 'Surface' refers to a surface to paint on, which is tied to a window
- 'SwapChain': a set of framebuffers swapped between

Information about vulkan:
In order to reconfigure the desired vulkan extensions, click the link in the glad auto generated vulkan.h file.
