#!/usr/bin/env python3
#
# Copyright 2021 The GPGMM Authors
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

import glob
import subprocess
import sys
import os
import re
import json

base_path = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))

binary_name = 'dawn_end2end_tests'
if sys.platform == 'win32':
    binary_name += '.exe'

build_path = os.path.join("out", "debug")
binary_path = os.path.join(base_path, build_path, binary_name)

vendor_id = "0x8086"
backend = "d3d12"
extra_args = ["--backend=" + backend, '--adapter-vendor-id=' + vendor_id]

process = subprocess.Popen([binary_path] + extra_args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
output, err = process.communicate()
data = output.decode("utf-8")

dict_resource_rej_count = {}
dict_test_rej_count = {}

curr_test_name = ""
for line in data.split("\r\n"):
  test_name_regex = re.search(r'RUN.*\] (.*)\/', line)
  if test_name_regex:
    curr_test_name = test_name_regex.group(1)

  rej_resource_desc = ""
  rej_resource_desc_regex = re.search(r'.*Resource alignment.*resource : ({.*})', line)
  if rej_resource_desc_regex:
    rej_resource_desc = rej_resource_desc_regex.group(1)
  else:
    continue # skip

  if rej_resource_desc not in dict_resource_rej_count:
      dict_resource_rej_count[rej_resource_desc] = 0
  dict_resource_rej_count[rej_resource_desc] += 1

  if curr_test_name not in dict_test_rej_count:
      dict_test_rej_count[curr_test_name] = 0
  dict_test_rej_count[curr_test_name] += 1

# Occurrence by resource request
print("Unique resources alignment rejected: " + str(len(dict_resource_rej_count)))
print("")

if not len(dict_resource_rej_count):
  sys.exit(0)

# Occurrence by test.
print("Occurrences by test:")
for test_name, count in sorted(dict_test_rej_count.items(), key=lambda x:x[1]):
  print(test_name + " (" + str(count) + ").")
print("")

# Occurrence by format
dict_resource_rej_format_count = {}
for resource_desc, count in dict_resource_rej_count.items():
  resource_desc_format = json.loads(resource_desc)['Format']
  if resource_desc_format not in dict_resource_rej_format_count:
      dict_resource_rej_format_count[resource_desc_format] = 0
  dict_resource_rej_format_count[resource_desc_format] += 1

print("Total occurrences (by format):")
for format_id, count in sorted(dict_resource_rej_format_count.items(), key=lambda x:x[1]):
  print("Format ID:" + str(format_id) + " (" + str(count) + ").")
print("")
