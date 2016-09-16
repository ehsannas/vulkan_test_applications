#!/usr/bin/python
# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""This contains functions determining element offsets in a structure.

Executing this file directly will run its tests.
This only works for single-nested structures. This also only
handles arrays of scalars, and not arrays of arrays of elements.
"""
import sys
# These values are meant to be used with the struct layout functions
UINT32_T, SIZE_T, POINTER, HANDLE, FLOAT, CHAR, ARRAY = range(7)

# These represents all of the possible states for parsing the struct offsets.
ELEMENT = 0
ARRAY_SIZE = 1
ARRAY_MEMBER = 2


def get_size_alignment(architecture):
    """Returns a dictionary of struct tags to tuples of size and
    alignment requirements."""
    return {
        UINT32_T: (4, 4),
        SIZE_T: (architecture.int_PointerSize, architecture.int_PointerSize),
        POINTER: (architecture.int_PointerSize, architecture.int_PointerSize),
        HANDLE: (8, architecture.extra_FieldAlignments.int_U64Alignment),
        FLOAT: (4, 4),
        CHAR: (1, 1)
    }


def align_to_next(number, alignment):
    """Returns number if number % alignment is 0 or returns the next
    number such that number % alignment is 0"""
    if number % alignment == 0:
        return number
    return number + alignment - (number % alignment)


class VulkanStruct:
    """Represents a vulkan structure. Given an array of element tags
    allows querying the offset of any individual element.

    If there is an array type, then the elements in the array
    should be ARRAY, array_size, TYPE_TAG
    """

    def __init__(self, architecture, elements):
        self.offsets = []
        self.generate_struct_offsets(architecture, elements)

    def get_offset_of(self, element):
        return self.offsets[element]

    def generate_struct_offsets(self, architecture, elements):
        state = ELEMENT
        array_size = 0
        offset = 0
        sizes_and_alignments = get_size_alignment(architecture)

        for element in elements:
            if state is ELEMENT:
                if element is ARRAY:
                    state = ARRAY_SIZE
                    continue
                size_and_alignment = sizes_and_alignments[element]
                offset = align_to_next(offset, size_and_alignment[1])
                self.offsets.append(offset)
                offset += size_and_alignment[0]
                continue
            if state is ARRAY_SIZE:
                array_size = element
                state = ARRAY_MEMBER
                continue
            if state is ARRAY_MEMBER:
                size_and_alignment = sizes_and_alignments[element]
                offset = align_to_next(offset, size_and_alignment[1])
                self.offsets.append(offset)
                offset += size_and_alignment[0] * array_size
                continue


def expect_eq(struct_name, pointer_size, u64_alignment, struct_elements,
              offsets):
    """Creates a VulkanStruct with the given pointer_size, u64_alignment, and
    elements.

    Checks that the offsets calculated are the same as the offsets
    expected.

    A message will be printed identifying the test and the result.
    If there is a failure, then it will return false.
    """
    # Create a mock architecture object. The only thing the object requires
    # is pointer_size
    test_name = struct_name + "." + str(pointer_size) + "." + str(u64_alignment)
    architecture = type("Architecture", (),
                        {"int_PointerSize": pointer_size,
                         "extra_FieldAlignments": type("FieldAlignments", (), {
                             "int_U64Alignment": u64_alignment
                         })})
    struct = VulkanStruct(architecture, struct_elements)
    success = True
    print "[ " + "RUN".ljust(10) + " ] " + test_name
    for i in range(0, len(offsets)):
        if struct.get_offset_of(i) != offsets[i]:
            print "Error: Element " + str(
                i) + " did not have the expected offset"
            print "   Expected: [" + str(offsets[i]) + "] but got [" + str(
                struct.get_offset_of(i)) + "]"
            success = False
    if success:
        print "[ " + "OK".rjust(10) + " ] " + test_name
        return True
    else:
        print "[ " + "FAIL".rjust(10) + " ] " + test_name
        return False


def test_structs(name, elements, x86_offsets, x86_64_offsets, armv7a_offsets,
                 arm64_offsets):
    """Runs expect_eq with the given test name for  each of the given
    architectures."""
    success = True
    success &= expect_eq(name + ".x86", 4, 4, elements, x86_offsets)
    success &= expect_eq(name + ".x64_64", 8, 8, elements, x86_64_offsets)
    success &= expect_eq(name + ".armv7a", 4, 8, elements, armv7a_offsets)
    success &= expect_eq(name + ".arm64", 8, 8, elements, arm64_offsets)
    return success


def main():
    success = True
    success &= test_structs(
        "uint32_struct", [UINT32_T],
        x86_offsets=[0],
        x86_64_offsets=[0],
        armv7a_offsets=[0],
        arm64_offsets=[0])

    success &= test_structs(
        "2uint32_struct", [UINT32_T, UINT32_T],
        x86_offsets=[0, 4],
        x86_64_offsets=[0, 4],
        armv7a_offsets=[0, 4],
        arm64_offsets=[0, 4])

    success &= test_structs(
        "pointer_struct", [UINT32_T, POINTER, UINT32_T],
        x86_offsets=[0, 4, 8],
        x86_64_offsets=[0, 8, 16],
        armv7a_offsets=[0, 4, 8],
        arm64_offsets=[0, 8, 16])

    success &= test_structs(
        "size_t_struct", [SIZE_T, POINTER, UINT32_T],
        x86_offsets=[0, 4, 8],
        x86_64_offsets=[0, 8, 16],
        armv7a_offsets=[0, 4, 8],
        arm64_offsets=[0, 8, 16])

    success &= test_structs(
        "handle_struct", [UINT32_T, HANDLE, SIZE_T, HANDLE],
        x86_offsets=[0, 4, 12, 16],
        x86_64_offsets=[0, 8, 16, 24],
        armv7a_offsets=[0, 8, 16, 24],
        arm64_offsets=[0, 8, 16, 24])

    success &= test_structs(
        "char_struct", [CHAR, CHAR, UINT32_T, CHAR],
        x86_offsets=[0, 1, 4, 8],
        x86_64_offsets=[0, 1, 4, 8],
        armv7a_offsets=[0, 1, 4, 8],
        arm64_offsets=[0, 1, 4, 8])

    success &= test_structs(
        "array_struct_middle", [UINT32_T, ARRAY, 4, UINT32_T],
        x86_offsets=[0, 4],
        x86_64_offsets=[0, 4],
        armv7a_offsets=[0, 4],
        arm64_offsets=[0, 4])

    success &= test_structs(
        "array_struct_start", [ARRAY, 3, UINT32_T, SIZE_T],
        x86_offsets=[0, 12],
        x86_64_offsets=[0, 16],
        armv7a_offsets=[0, 12],
        arm64_offsets=[0, 16])
    return success


if __name__ == "__main__":
    sys.exit(0 if main() else -1)
