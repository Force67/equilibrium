# Base mission statement

Our mission is to build a library that covers all the basic needs of a cxx application in a portable and uniform way. 

*Most importantly, we want to:*
- cover all basic needs of a modern cxx application and then some
- aim for maximum test coverage
- use _sensible_ modern cxx, without any STL whereever possible
- have speed by default as our goal
- have the library be customizeable, so you can disable the parts you don't like or need

In general, every class should have a unit test file assigned to it. Files that do should be safe to use in production.