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
'''The main test framework for all Gapit tests.
'''

import argparse
import fnmatch
import inspect
import os
import re
import tempfile
import shutil
import sys
import traceback

from gapit_tester import run_on_single_apk
from gapit_tester import RunArgs
from gapit_trace_reader import parse_trace_file, AtomAttributeError

SUCCESS = 0
FAILURE = 1


class GapitTestException(Exception):
    """Base test exception"""
    pass


def little_endian_bytes_to_int(val):
    '''Takes a sequence of bytes in little-endian format, and turns
    them into an integer'''
    total = 0
    place = 1
    for byte in val:
        total = total + (byte * place)
        place *= 256
    return total


def require(val):
    '''Takes a tuple (Object, String).

    If Object is None, throws an exception with the string and fails the test,
    otherwise, prints nothing and returns the Object
    '''
    if val[0] is not None:
        return val[0]
    else:
        call_site = traceback.format_list(traceback.extract_stack(limit=2))
        raise GapitTestException(val[1] + "\n" + call_site[0])


def require_equal(param, val):
    """Takes 2 values. If they are equal, does nothing, otherwise
    throws an exception with an error message"""
    if type(val)(param) == val:
        return
    call_site = traceback.format_list(traceback.extract_stack(limit=2))
    raise GapitTestException("Expected:" + str(param) + "==" + str(val) + "\n" +
                             call_site[0])


def require_not_equal(param, val):
    """Takes 2 values. If they are not equal, does nothing, otherwise
    throws an exception with an error message"""
    if type(val)(param) != val:
        return
    call_site = traceback.format_list(traceback.extract_stack(limit=2))
    raise GapitTestException("Expected:" + str(param) + "==" + str(val) + "\n" +
                             call_site[0])


def require_not(val):
    '''Takes a tuple (Object, String). If Object is not None, throws an
    exception stating that we expected this to not exist. Otherwise does
    nothing'''
    if val[0] is not None:
        call_site = traceback.format_list(traceback.extract_stack(limit=2))
        raise GapitTestException("Found element we were not expecting\n" +
                                 call_site[0])


class GapitTest(object):
    '''Base class for all Gapit tests.

    This is responsbile for tracing our application as well as processing the
    log file. Any base-classes 'expect' method will be run.
    '''

    def __init__(self):
        self.atom_generator = None

    def name(self):
        """Returns the name of this class."""
        return self.__class__.__name__

    def next_call_of(self, call_name):
        '''Consumes atoms until a call of the given type is found, returns
        a tuple (Atom, ""). Returns (None, "error_message") if the atom could
        not be found'''
        try:
            while True:
                atom = self.atom_generator.next()
                if atom.name == call_name:
                    return (atom, "")
        except StopIteration:
            return (None, "Could not find atom of type " + call_name)

    def nth_call_of(self, call_name, index):
        '''Consumes atoms until the nth call of the given type is found,
           returns a tuple (Atom, ""). Returns (None, "error_message") if
           the atom could not be found'''
        atom = None
        for _ in range(0, index):
            atom = self.next_call_of(call_name)
        if atom:
            return (atom, "")
        else:
            return (None,
                    "Could not find " + index + " atoms of type " + call_name)

    def next_call(self, call_name):
        '''Expects the next call to be of the specified type. Returns
        a tuple (Atom, ""), if it could be found, otherwise returns
        (None, "Error_message")
        '''
        try:
            atom = self.atom_generator.next()
            if atom.name == call_name:
                return (atom, "")
        except StopIteration:
            return (None, "The next atom was not of type " + call_name)

    def run_test(self, verbose, capture_directory):
        '''Runs this test case. Returns a tuple, (FAILURE|SUCCESS, message) on
           completion.'''
        apk_name = getattr(self, "gapit_test_apk_name")
        capture_name = os.path.join(capture_directory, apk_name + ".gfxtrace")
        if not os.path.isfile(capture_name):
            # If the file has not been created yet, in this test run, then
            # generate the capture file contents. This is a slow process so
            # only do it once.
            args = RunArgs()
            if verbose:
                args.set_verbose()
            args.set_output(capture_name)
            setattr(args, "verbose", verbose)
            setattr(args, "keep", False)
            setattr(args, "output", [capture_name])
            print "[ " + "TRACING".center(10) + " ] " + apk_name
            return_value = run_on_single_apk(apk_name, args)
            if return_value != 0:
                return (FAILURE, "Could not generate trace file.")
            if verbose:
                print "Successfully traced application"
            print "[ " + "DONE".center(10) + " ] " + apk_name
        test_name = apk_name + "." + self.name()
        self.atom_generator = parse_trace_file(capture_name)
        print "[ " + "RUN".ljust(10) + " ] " + test_name
        try:
            getattr(self, "expect")()
        except GapitTestException as error:
            print "[ " + "FAILED".rjust(10) + " ] " + test_name
            print "     " + error.message
            return (FAILURE, error.message)
        except AtomAttributeError as error:
            exc_type, exc_value, exc_tb = sys.exc_info()
            print "[ " + "FAILED".rjust(10) + " ] " + test_name
            call_site = traceback.format_exception(
                exc_type, exc_value, exc_tb, limit=2)
            print "    " + error.message + call_site[1]
        print "[ " + "OK".rjust(10) + " ] " + test_name
        return (SUCCESS, None)


def gapit_test(gapit_test_apk_name):
    ''' This is used as a decoration to classes.

        It checks that the class it is attached to is of the right type,
        and sets the apk_name on it.'''

    def checked_decorator(clss):
        """ This actually performs all of the checks"""
        if not inspect.isclass(clss):
            raise GapitTestException("The test case should be a class")
        if not issubclass(clss, GapitTest):
            raise GapitTestException(
                "The test case should be a subclass of GapitTest")
        clss.gapit_test_apk_name = gapit_test_apk_name
        return clss

    return checked_decorator


class TestManager(object):
    """Manages and runs all requested tests."""

    def __init__(self, root_directory, verbose):
        """root_directory is the directory that contains all of the tests"""
        self.root_directory = root_directory
        self.verbose = verbose
        self.tests = {}
        self.number_of_tests_run = 0
        self.failed_tests = []

    def gather_all_tests(self):
        '''Finds all classes in all python files that contain the attribute
            @gapit_test'''
        default_path = sys.path
        for root, _, filenames in os.walk(self.root_directory):
            for filename in fnmatch.filter(filenames, "*.py"):
                sys.path = default_path
                sys.path.append(root)
                if self.verbose:
                    print("Searching " + os.path.join(root, filename) +
                          " for tests")
                mod = __import__(os.path.splitext(filename)[0])
                for _, obj in inspect.getmembers(mod):
                    if inspect.isclass(obj) and hasattr(obj,
                                                        "gapit_test_apk_name"):
                        if self.verbose:
                            print("Found test " + obj.__name__ + " in " +
                                  os.path.join(root, filename))
                        self.add_test(obj.gapit_test_apk_name, obj())

    def print_test_names(self, file_handle):
        """Prints out to the given file handle a list of all tests."""
        for apk, tests in self.tests.iteritems():
            for test in tests:
                file_handle.write(apk + "." + test.name() + "\n")

    def add_test(self, apk_name, test):
        """Adds a test case to the manager from the given apk"""
        if not apk_name in self.tests:
            self.tests[apk_name] = []
        self.tests[apk_name].append(test)

    def run_all_tests(self, temp_directory, include_regex, exclude_regex):
        '''Runs all of the tests that match the given regular expression
        string'''
        include_matcher = re.compile(include_regex)
        exclude_matcher = re.compile(exclude_regex)
        for apk in sorted(self.tests.keys()):
            for test in sorted(self.tests[apk], key=lambda x: x.name()):
                if (include_matcher.match(apk + "." + test.name()) and
                        None == exclude_matcher.match(apk + "." + test.name())):
                    self.accumulate_result(
                        apk + "." + test.name(),
                        test.run_test(self.verbose, temp_directory))

    def accumulate_result(self, test_name, result):
        """Tracks the results for the given test"""
        self.number_of_tests_run += 1
        if result[0] == FAILURE:
            self.failed_tests.append((test_name, result[1]))

    def print_summary_and_return_code(self):
        '''Prints a summary of the test results. Returns 0 if every test
        was successful and -1 if any test failed'''
        print "Total Tests Run: " + str(self.number_of_tests_run)
        print("Total Tests Passed: " +
              str(self.number_of_tests_run - len(self.failed_tests)))
        for failure in self.failed_tests:
            print "[ " + "FAILED".rjust(10) + " ] " + failure[0]
            print "     " + failure[1]
        if self.failed_tests:
            return -1
        return 0


def main():
    """Entry-point for the testing framework"""
    parser = argparse.ArgumentParser(description='''
This testing framework will look in a given directory and accumulate
tests from there.

Any class that is a subclass of GapitTest and has the annotation @gapit_test
will be considered.

It is expected the CWD of this script is the output directory for the APK files
from a build.
        ''')
    parser.add_argument(
        "--test-dir", nargs=1, help="Directory to gather the tests from")
    parser.add_argument(
        "--verbose", action="store_true", help="Show verbose output")
    parser.add_argument(
        "--list-tests",
        action="store_true",
        help="Only print the names of all tests")
    parser.add_argument(
        "--include",
        default=".*",
        help="Run tests matching this regular expression")
    parser.add_argument(
        "--exclude",
        default="^$",
        help="Exclude tests that match this regular expression")
    args = parser.parse_args()

    test_directory = os.getcwd()
    if args.test_dir:
        test_directory = args.test_dir[0]

    manager = TestManager(test_directory, args.verbose)
    manager.gather_all_tests()

    if args.list_tests:
        manager.print_test_names(sys.stdout)
        return 0
    else:
        test_directory = tempfile.mkdtemp()
        manager.run_all_tests(test_directory, args.include, args.exclude)
        shutil.rmtree(test_directory)
        return manager.print_summary_and_return_code()


if __name__ == "__main__":
    sys.exit(main())
