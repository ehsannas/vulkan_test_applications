#!/usr/bin/python

'''This runs a single apk through gapit trace and gapit dump.'''

import argparse
import os
import subprocess
import sys
import android


def run_on_single_apk(apk, args):
    '''Installs, traces and optionally uninstalls a single APK from an android device.'''
    apk_info = android.get_apk_info(apk)
    android.install_apk(apk, args)
    android.adb(['logcat', '-c'], args)

    gapit_args = ['gapit']
    if args.verbose:
        gapit_args.extend(['-log-level' 'Debug'])
    gapit_args.extend(['trace', apk_info.package_name])
    p = subprocess.Popen(gapit_args)
    return_value = android.watch_process(True, args)
    android.adb(['shell', 'am', 'force-stop', apk_info.package_name], args)
    if not args.keep:
        android.adb(['uninstall', apk_info.package_name], args)

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

