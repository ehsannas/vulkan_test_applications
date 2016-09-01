#!/usr/bin/python

'''This contains useful functions for talking to android devices and getting
information about .apks'''

import collections
import os
import subprocess
import sys

def adb(params, program_args):
    '''Runs a single command through ADB.

    Arguments:
        params: A list of the parameters to pass to adb
        program_args: The arguments to this program.

        program_args must contain a .verbose member.

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

        program_args must contain a .verbose member.

        If program_args.verbose is true, then the command is printed.
    '''
    args = ['adb']
    args.extend(params)
    if program_args.verbose:
        print args
    return subprocess.Popen(args, stdout=subprocess.PIPE)

def install_apk(apk, program_args):
    '''Installs an apk. Overwrites existing APK if it exists and grants all permissions.

        program_args must have a .verbose member.

    '''
    adb(['install', '-r', '-g', apk], program_args)


def get_apk_info(apk):
    ''' Returns a named tuple (test_name, package_name, activity_name) for the given apk.'''
    test_name = os.path.splitext(os.path.basename(apk))[0]
    package_name = 'com.example.test.' + test_name
    activity_name = 'android.app.NativeActivity'
    apk_info = collections.namedtuple('ApkInfo', ['test_name', 'package_name', 'activity_name'])
    return apk_info(test_name, package_name, activity_name)

def watch_process(silent, program_args):
    ''' Watches the output of a running android process.

    Arguments:
        silent: True if output should be consumed, false otherwise
        program_args: The arguments passed to the program.

        program_args must contain a .verbose member.

    It is expected that the log was cleared before the process started.

    Returns the return code that it produced

    '''
    p = adb_stream(['logcat', '-s', '-v', 'brief', 'VulkanTestApplication:V'], program_args)
    return_value = 0

    while True:
        line = p.stdout.readline()
        if line != '':
            split_line = line.split(':')[1:]
            if not split_line:
                continue
            line_text = ':'.join(split_line)[1:]
            if split_line and split_line[0] == ' RETURN':
                if program_args.verbose:
                    print line_text,
                return_value = int(split_line[1])
                break
            if program_args.verbose or not silent:
                print line_text,
    return return_value
