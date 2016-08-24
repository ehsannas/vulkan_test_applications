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

- [cmake](cmake/README.md)
- [support](support/README.md)
- [vulkan_function_loader](vulkan_function_loader/README.md)