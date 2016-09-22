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
"""This contains functions for parsing a trace file using gapit dump."""

import argparse
import re
import subprocess
import sys


class Observation(object):
    '''A single observation.

    Contains the start, end and id of the observation as well as
    the memory that was actually observed.'''

    def __init__(self, memory_start, memory_end, memory_id):
        self.contents = []
        self.memory_start = memory_start
        self.memory_end = memory_end
        self.memory_id = memory_id

    def get_memory_range(self):
        """ Returns a pair (start, end) of the memory region"""
        return (self.memory_start, self.memory_end)

    def get_memory(self):
        """ Returns the bytes inside the range"""
        return self.contents


def find_memory_in_observations(observations, address, num_bytes):
    '''Searches for the given range of bytes in the given array of observations.

    Returns the bytes if they exist, or returns None if they could not be
    found
    '''

    for observation in observations:
        rng = observation.get_memory_range()
        last_address = address + num_bytes - 1
        if (address <= rng[1] and address >= rng[0] and
                last_address <= rng[1] and last_address >= rng[0]):
            return observation.get_memory()[address - rng[0]:address - rng[0] +
                                            num_bytes]
    return None


def find_string_in_observations(observations, address):
    '''Finds a null-terminated string starting at the given address in
    the observations. Returns a python string containing all of the non
    null bytes, or None if it could not be found'''
    for observation in observations:
        rng = observation.get_memory_range()
        if address <= rng[1] and address >= rng[0]:
            memory = observation.get_memory()
            string_data = bytearray()
            for idx in range(address - rng[0], rng[1] - rng[0] + 1):
                ch = memory[idx]
                if ch is 0:
                    return str(string_data)
                string_data.append(ch)
    return None


class NamedAttributeError(AttributeError):
    """An exception that is thrown when trying to access a parameter from an
    object"""

    def __init__(self, message):
        super(NamedAttributeError, self).__init__(message)


class Extra(object):
    """A single extra attached to an atom"""

    def __init__(self, name, parameters):
        self.name = name
        self.parameters = {}
        for parameter in parameters:
            self.parameters[parameter[0]] = parameter[1]

    def __getattr__(self, name):
        """Returns parameters that were on this extra.

        Based on the prefix (hex_, int_, <none>) we convert the parameter
        from a string with the given formatting.
        """

        if name.startswith('hex_') or name.startswith('int_'):
            if name[4:] in self.parameters:
                if name.startswith('hex_'):
                    return int(self.parameters[name[4:]], 16)
                elif name.startswith('int_'):
                    return int(self.parameters[name[4:]])

        if name in self.parameters:
            return self.parameters[name]
        raise NamedAttributeError('Could not find parameter ' + name +
                                  ' on extra ' + self.name + '\n')


class Atom(object):
    '''A single Atom that was observed.

    Contains all of the memory observations, parameters, index and return code
    of the atom.'''

    def __init__(self, index, name, return_val):
        self.parameters = {}
        self.read_observations = []
        self.write_observations = []
        self.index = index
        self.name = name
        self.return_val = return_val
        self.extras = {}

    def __getattr__(self, name):
        """Returns parameters that were on this atom.

        Based on the prefix (hex_, int_, <none>) we convert the parameter
        from a string with the given formatting.
        """

        if name.startswith('hex_') or name.startswith('int_'):
            if name[4:] in self.parameters:
                if name.startswith('hex_'):
                    return int(self.parameters[name[4:]], 16)
                elif name.startswith('int_'):
                    return int(self.parameters[name[4:]])

        if name in self.parameters:
            return self.parameters[name]
        if name.startswith('extra_'):
            if name[6:] in self.extras:
                return self.extras[name[6:]]

        raise NamedAttributeError('Could not find parameter ' + name +
                                  ' on atom [' + str(self.index) + ']' +
                                  self.name + '\n')

    def num_observations(self):
        """Returns the number of observations on this object"""
        return len(self.read_observations) + len(self.write_observations)

    def get_read_data(self, address, num_bytes):
        """Returns num_bytes from the starting address in the read observations.

        If the range address->address + num_bytes is not contained in a read
        Observation (None, "Error_Message") is returned, otherwise (bytes, "")
        is returned
        """
        mem = find_memory_in_observations(self.read_observations, address,
                                          num_bytes)
        if mem is not None:
            return (mem, '')

        return (None, 'Could not find a read observation starting at ' +
                str(hex(address)) + ' containing ' + str(num_bytes) + ' bytes')

    def get_read_string(self, address):
        """Returns a null-terminated string starting at address in the
        read observations.

        If there is no null-terminated string at address, then
        (None, "Error_Message") is returned otherwise (string, "")
        is returned"""
        mem = find_string_in_observations(self.read_observations, address)
        if mem is not None:
            return mem, ''

        return (None, 'Could not find a string starting at ', address)

    def get_write_data(self, address, num_bytes):
        """Returns num_bytes from the starting address in the write
        observations.

        If the range address->address + num_bytes is not contained in a write
        Observation (None, "Error_Message") is returned, otherwise (bytes, "")
        is returned
        """
        mem = find_memory_in_observations(self.write_observations, address,
                                          num_bytes)
        if mem is not None:
            return (mem, '')
        else:
            return (None, 'Could not find a read observation starting at ',
                    address, ' containing ', num_bytes, ' bytes')

    def get_write_string(self, address):
        """Returns a null-terminated string starting at address in the
        read observations.

        If there is no null-terminated string at address, then
        (None, "Error_Message") is returned otherwise (string, "")
        is returned"""
        mem = find_string_in_observations(self.write_observations, address)
        if mem is not None:
            return mem, ''

        return (None, 'Could not find a string starting at ', address)

    def add_parameter(self, name, value):
        """Adds a parameter with the given name and value to the atom."""
        self.parameters[name] = value

    def get_parameter(self, parameter_name):
        '''Expects there to be a paramter of the given name on the given atom.
        Returns (parmeter, "") if it exists, and (None, "Error_message") if it
        does not'''
        if parameter_name in self.parameters:
            return (self.parameters[parameter_name], '')
        else:
            return (None, 'Could not find paramter ' + parameter_name +
                    ' on atom [' + str(self.index) + '] ' + self.name())

    def add_read_observation(self, memory_start, memory_end, memory_id):
        '''Adds a read observation to the atom. Does not associate any memory
        with the read'''
        self.read_observations.append(
            Observation(memory_start, memory_end, memory_id))

    def add_write_observation(self, memory_start, memory_end, memory_id):
        '''Adds a write observation to the atom. Does not associate any memory
        with the write'''
        self.write_observations.append(
            Observation(memory_start, memory_end, memory_id))

    def add_extra(self, name, parameters):
        """Takes a name and an array of parameter tuples (name, value) and adds
        this extra to the atom"""
        self.extras[name] = Extra(name, parameters)

    def set_read_observation_bytes(self, index, memory):
        """Adds bytes to the read observation with the given index"""
        self.read_observations[index].contents = memory

    def set_write_observation_bytes(self, index, memory):
        """Adds bytes to the write observation with the given index"""
        self.write_observations[index].contents = memory


def parse_atom_line(line):
    '''Parses a single line from a trace dump.

    Example lines:
    000000 switchThread(ThreadID: 1)
    000036 vkGetDeviceProcAddr(Device: 32, PName: vkMapMemory)-> 0xd83ebcac

    This is expected to be first line of a new atom
    '''
    match = re.match(r'(\d+) ([a-zA-Z]*)\((.*)\)(?:->(.*))?\n', line)
    number = int(match.group(1))
    command = match.group(2)

    all_parameters = match.group(3)
    parameters = re.findall(r'([a-zA-Z]*): ([a-zA-Z0-9]*)', all_parameters)

    return_val = None
    if match.group(4):
        return_val = match.group(4)

    atom = Atom(number, command, return_val)
    for parameter in parameters:
        atom.add_parameter(parameter[0], parameter[1])

    return atom


def parse_memory_observations(observation):
    '''Parses all memory observations from a formatted list of observations.

    Example Line:
    {Range: [0x00000000eaafff34-0x00000000eaafff37],                           \
        ID: 8f85530ad0db61eee69aa0ba583751043584d500}

    Returns a list of tuples containing the range start, range end, and ID of
    the observation.
    '''
    observations = re.findall(
        r'\{Range: \[0x([0-9a-f]+)-0x([0-9a-f]+)\], ID: ([0-9a-f]+)\}',
        observation)
    for observation in observations:
        yield observation


def parse_extra(name, extra_text, atom):
    """Parses extra_text from an extra named `extra_name`


    Example extra_text:
    &{Generate:{} U64Alignment: 8}
    """
    interior = extra_text.lstrip('&').strip('{}')
    parameters = re.findall(r'([a-zA-Z0-9]+):(\S)+', interior)
    atom.add_extra(name, parameters)


def parse_observations(line, atom):
    '''Parses the second line of an atom.

    Rerturns the number of read and write
    observations. Adds the observations to the given atom.

    Example Lines:
    Reads: [], Writes: []

    Reads: [], Writes: [{Range: [0x00000000eaafff34-0x00000000eaafff37],       \
                          ID: 8f85530ad0db61eee69aa0ba583751043584d500}]
    '''
    match = re.match(r'Reads: \[(.*)\], Writes: \[(.*)\]', line)
    num_read_observations = 0
    num_write_observations = 0
    if match.group(1):
        for observation in parse_memory_observations(match.group(1)):
            atom.add_read_observation(
                int(observation[0], 16), int(observation[1], 16),
                observation[2])
            num_read_observations += 1
    if match.group(2):
        for observation in parse_memory_observations(match.group(2)):
            atom.add_write_observation(
                int(observation[0], 16), int(observation[1], 16),
                observation[2])
            num_write_observations += 1
    return (num_read_observations, num_write_observations)


def parse_memory_line(line):
    """Parses the memory lines of an atom, returns the bytes of the
    observation

    [0 23 254 0]
    """
    match = re.match(r'\s*\[([0-9a-f ]*)\]', line)
    return [int(x, 10) for x in match.group(1).split(' ')]

# These are the states that we use when parsing.
# FIRST_LINE is the first line in the entire file
FIRST_LINE = 1
# NEW_ATOM is the state we are in just before reading a new atom
NEW_ATOM = 2
# EXTRAS is the state we are in when we have finished reading an Atom
# and we are looking for the Extras
EXTRAS = 3
# MEMORY is the state we are in when we have finished reading the EXTRAS
# and now we have to read all of the bytes of memory
MEMORY = 4


def parse_trace_file(filename):
    '''Parses a trace file, for every atom parsed, yields an atom.

    Arguments:
        filename is the name of the trace file to read
    '''
    proc = subprocess.Popen(
        ['gapit', 'dump', '-observations', filename], stdout=subprocess.PIPE)

    # This parsing routine is basically a state machine
    # The very first line of the process is un-necessary
    # Every atom has 3 parts:
    # The first line is the number/name/parameters
    # The second line is information about the read/write locations
    # The subsequent lines are the memory contents of those read/write locations
    #   IMPORTANT: These memory contents are what is present AFTER the
    #              call is made, not before. So you cannot get information
    #              about what the memory was before a call.
    num_read_observations = 0
    num_write_observations = 0
    current_memory_observation = 0
    current_state = FIRST_LINE
    atom = None
    # readline keeps the newline on the end so EOF is when readline returns
    # an empty string
    line = proc.stdout.readline()
    while line != '':
        if current_state == FIRST_LINE:
            current_state = NEW_ATOM
        elif current_state == NEW_ATOM:
            if atom:
                yield atom
            atom = parse_atom_line(line)
            current_state = EXTRAS
        elif current_state == EXTRAS:
            match = re.match(r'\s*([a-zA-Z]+): \[(.*)\]', line)
            if not match:
                # If there were no memory obeservations made, then
                # we actively do not want to consume this line, this
                # is the start of a new atom, and we should treat it as
                # such
                if not atom.num_observations():
                    current_state = NEW_ATOM
                else:
                    current_state = MEMORY
                continue
            if match.group(1) == 'Observations':
                observations = parse_observations(match.group(2), atom)
                num_read_observations = observations[0]
                num_write_observations = observations[1]
            else:
                parse_extra(match.group(1), match.group(2), atom)
        elif current_state == MEMORY:
            memory = parse_memory_line(line)
            if current_memory_observation < num_read_observations:
                atom.set_read_observation_bytes(current_memory_observation,
                                                memory)
            else:
                atom.set_write_observation_bytes(
                    current_memory_observation - num_read_observations, memory)

            current_memory_observation += 1
            total_observations = num_read_observations + num_write_observations
            if current_memory_observation == total_observations:
                current_state = NEW_ATOM
                current_memory_observation = 0
        line = proc.stdout.readline()


def main():
    """Main entry point for the parser."""
    parser = argparse.ArgumentParser(
        description='Prints out the name of all atoms found in a trace file')
    parser.add_argument('trace', help='trace file to read')
    args = parser.parse_args()

    for atom in parse_trace_file(args.trace):
        print atom.name
    return 0


if __name__ == '__main__':
    sys.exit(main())
