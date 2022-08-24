# Entitlement Component

A set of tools for issuing entilements e.g authorization for using a certain product and given feature set. In our nomenclature we refer to those as ```keystones```. This component is considered private and closed source. The entilement bits are also integrated with NEMISIS for additional obfuscation. Logging is turned off for release builds among with asserts.
```
├───issuing_tool < generate licenses and keys
├───keystone < implementation of the library used for dealing with licenses and keys
├───licensee_database_server < microservice driving the data storage of licensees
├───protocol < specification for sending data between licensee_database and others
├───safe_crypto < wrapper around crypto library with test coverage
└───validation_server < backend component that will handle license validation requests
```

