# Android Vulkan Tests

To build apk files.

```
cmake -GNinja {root} -DBUILD_APKS=ON -DANDROID_SDK=path/to/android/sdk
```

This assumes the android ndk is installed in the default location of
path/to/android/sdk/ndk-bundle.

If it is installed elsewhere, use
```
cmake -GNinja {root} -DBUILD_APKS=ON -DANDROID_SDK=path/to/android/sdk -DANDROID_NDK=path/to/ndk
```

To build only for 32-bit ARM platform.
```
cmake -GNinja {root} -DBUILD_APKS=ON -DANDROID_SDK=path/to/android/sdk -DANDROID_ABIS=armeabi-v7a
```

# Support Functionality
- [cmake](cmake/README.md)
- [support](support/README.md)
- [vulkan_wrapper](vulkan_wrapper/README.md)
- [vulkan_helpers](vulkan_helpers/README.md)

# Tests
- [command_buffer](command_buffer_tests/README.md)
- [initialization](initialization_tests/README.md)
- [resource creation](resource_creation_tests/README.md)
- [traits query](traits_query_tests/README.md)
