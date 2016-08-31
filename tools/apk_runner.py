#!/usr/bin/python

'''This runs a given APK that was compiled from this repository.

This runs a given APK on the connected Android device. It makes assumptions
about log output and package names that means it is only useful
for this repository
'''

import argparse
import os
import subprocess
import sys

def adb(params, program_args):
    '''Runs a single command through ADB.

    Arguments:
        params: A list of the parameters to pass to adb
        program_args: The arguments to this program.

        If program_args.verbose is true, then the command and the output is printed,
        otherwise no output is present.
    '''
    args = ['adb']
    args.extend(params)
    if program_args.verbose:
        print args
        subprocess.check_call(args)
    else:
        subprocess.check_call(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def adb_stream(params, program_args):
    '''Runs a single command through ADB, returns the process with stdout redirected.

    Arguments:
        params: A list of the parameters to pass to adb
        program_args: The arguments to this program.

        If program_args.verbose is true, then the command is printed.
    '''
    args = ['adb']
    args.extend(params)
    if program_args.verbose:
        print args
    return subprocess.Popen(args, stdout=subprocess.PIPE)

def run_on_single_apk(apk, args):
    '''Installs, runs and optionally uninstalls a single APK from an android device.'''
    test_name = os.path.splitext(os.path.basename(apk))[0]
    package_name = 'com.example.test.' + test_name
    activity_name = 'android.app.NativeActivity'
    adb(['install', '-r', apk], args)
    adb(['logcat', '-c'], args)
    adb(['shell', 'am', 'start', '-n', package_name + '/' + activity_name], args)
    p = adb_stream(['logcat', '-s', '-v', 'brief', 'VulkanTestApplication:V'], args)
    return_value = 0

    while True:
        line = p.stdout.readline()
        # The line looks like VulkanTestApplication: <what you wrote in the log>
        if line != '':
            # Remove VulkanTestApplication:
            split_line = line.split(':')[1:]
            if not split_line:
                continue
            # Join any : back, and remove the leading space
            line_text = ':'.join(split_line)[1:]
            if split_line and split_line[0] == ' RETURN':
                if args.verbose:
                    print line_text,
                return_value = int(split_line[1])
                break
            print line_text,

    adb(['shell', 'am', 'force-stop', package_name], args)
    if not args.keep:
        adb(['uninstall', package_name], args)
    sys.exit(return_value)

def main():
    parser = argparse.ArgumentParser(
                description='Run a .apk file on an android device')
    parser.add_argument('apk', help='apk to run')
    parser.add_argument('--keep', action='store_true', help='do not uninstall on completion')
    parser.add_argument('--verbose',  action='store_true', help='enable verbose output')
    args = parser.parse_args()
    run_on_single_apk(args.apk, args)


if __name__ == '__main__':
    main()
