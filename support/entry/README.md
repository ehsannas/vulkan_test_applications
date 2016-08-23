The `entry` library is used to set up system agnostic entry-point routines.
This library creates a window that is suitable for Vulkan rendering. This
window is presented to the user, and passed to the `main_entry` in the
`entry_data` field as described below.

You must, somewhere in your program, define a function with the signature
`int main_entry(const entry_data* data);` This function will get called
by the entry point library.