## Test 00: Invalid /proc directory [1 pts]

If the directory given for -p is invalid, print an error message and exit.
Return EXIT_FAILURE or similar. Test fails if return = 0 or segfault.

```

./inspector -a -p /this/directory/does/not/exist
inspector.c:210:main(): Using alternative proc directory: /this/directory/does/not/exist
Problem opening directory: /this/directory/does/not/exist
[[ ${?} -ne 0 && ${?} -ne 139 ]];
test_end
```

## Test 01: System/Hardware/Task Headers [1 pts]

Tests that the various heading combinations are present.

```

reported_headings=$(./inspector -a \
    | grep -E 'System Information|Hardware Information|Task Information' | sort)
inspector.c:233:main(): View options selected: hardware system task_list
compare --ignore-all-space \
    <(echo "${expected_headings1}") <(echo "${reported_headings}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Hardware Information                                                     Hardware Information
System Information                                                       System Information
Task Information                                                         Task Information
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

reported_headings=$(./inspector -r \
    | grep -E 'System Information|Hardware Information|Task Information' | sort)
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space \
    <(echo "${expected_headings2}") <(echo "${reported_headings}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Hardware Information                                                     Hardware Information
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

reported_headings=$(./inspector -s \
    | grep -E 'System Information|Hardware Information|Task Information' | sort)
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space \
    <(echo "${expected_headings3}") <(echo "${reported_headings}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
System Information                                                       System Information
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

reported_headings=$(./inspector -t \
    | grep -E 'System Information|Hardware Information|Task Information' | sort)
inspector.c:233:main(): View options selected: task_list
compare --ignore-all-space \
    <(echo "${expected_headings4}") <(echo "${reported_headings}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Task Information                                                         Task Information
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

reported_headings=$(./inspector -ts \
    | grep -E 'System Information|Hardware Information|Task Information' | sort)
inspector.c:233:main(): View options selected: system task_list
compare --ignore-all-space \
    <(echo "${expected_headings5}") <(echo "${reported_headings}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
System Information                                                       System Information
Task Information                                                         Task Information
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK


test_end
```

## Test 02: Hostname Check [1 pts]

```

expected_hostname="Hostname: deus-ex-machina"
reported_hostname=$(./inspector -p "${TEST_DIR}/fakeproc" -s \
    | grep -i 'Hostname')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_hostname}") <(echo "${reported_hostname}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Hostname: deus-ex-machina                                                Hostname: deus-ex-machina
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

expected_hostname="Hostname: $(hostname)"
reported_hostname=$(./inspector -s | grep -i 'Hostname')
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_hostname}") <(echo "${reported_hostname}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Hostname: alhanson7210                                                   Hostname: alhanson7210
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 03: Kernel Version [1 pts]

```

expected_version="Kernel Version: 1.33.7-111.1.1.el7.x86_64"
reported_version=$(./inspector -p "${TEST_DIR}/fakeproc" -s | grep -i 'kernel')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_version}") <(echo "${reported_version}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Kernel Version: 1.33.7-111.1.1.el7.x86_64                                Kernel Version: 1.33.7-111.1.1.el7.x86_64
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

expected_version="Kernel Version: $(uname -r)"
reported_version=$(./inspector -s | grep -i 'Kernel')
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_version}") <(echo "${reported_version}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Kernel Version: 5.2.9-arch1-1-ARCH                                       Kernel Version: 5.2.9-arch1-1-ARCH
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 04: Basic Uptime Formatting [1 pts]

```

run ./inspector -p "${TEST_DIR}/fakeproc" -s
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: system 

filter 'Uptime'
 --> Filter matched 1 line(s)

compare --ignore-all-space --ignore-case \
    <(echo "${expected_uptime}") <(echo "${filtered_output}") || test_end

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Uptime: 17 days, 5 hours, 38 minutes, 55 seconds                         Uptime: 17 days, 5 hours, 38 minutes, 55 seconds
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 05: Uptime Formatting: Short Timespan [1 pts]

```

prev_uptime=$(cat "${TEST_DIR}/fakeproc/uptime")
echo "135.17 93892480.110" > "${TEST_DIR}/fakeproc/uptime"

expected_uptime="Uptime: 2 minutes, 15 seconds"
reported_uptime=$(./inspector -p "${TEST_DIR}/fakeproc" -s | grep -i 'Uptime')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: system 
echo "${prev_uptime}" > "${TEST_DIR}/fakeproc/uptime"

compare --ignore-all-space --ignore-case \
    <(echo "${expected_uptime}") <(echo "${reported_uptime}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Uptime: 2 minutes, 15 seconds                                            Uptime: 2 minutes, 15 seconds
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 06: Uptime Formatting: Long Timespan [1 pts]

```

prev_uptime=$(cat "${TEST_DIR}/fakeproc/uptime")
echo "126146404.41 93892480.110" > "${TEST_DIR}/fakeproc/uptime"

expected_uptime="Uptime: 4 years, 40 minutes, 4 seconds"
reported_uptime=$(./inspector -p "${TEST_DIR}/fakeproc" -s | grep -i 'Uptime')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: system 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_uptime}") <(echo "${reported_uptime}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Uptime: 4 years, 40 minutes, 4 seconds                                   Uptime: 4 years, 40 minutes, 4 seconds
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

echo "${prev_uptime}" > "${TEST_DIR}/fakeproc/uptime"

test_end
```

## Test 07: Processing Units [1 pts]

```

expected_units="Processing Units: 64"
reported_units=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Processing.*:')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_units}") <(echo "${reported_units}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Processing Units: 64                                                     Processing Units: 64
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

expected_units="Processing Units: $(grep '^processor' /proc/cpuinfo | wc -l)"
reported_units=$(./inspector -r | grep -i 'Processing.*:')
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_units}") <(echo "${reported_units}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Processing Units: 2                                                      Processing Units: 2
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 08: CPU Model [1 pts]

```

expected_model="CPU Model: AMD EPYC 7281 16-Core Processor"
reported_model=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'CPU[[:space:]]*Model')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_model}") <(echo "${reported_model}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
CPU Model: AMD EPYC 7281 16-Core Processor                               CPU Model: AMD EPYC 7281 16-Core Processor
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

expected_model="CPU Model: $(grep 'model name' /proc/cpuinfo \
    | sed 's/.*: //' | head -n 1 )"
reported_model=$(./inspector -r | grep -i 'CPU[[:space:]]*Model')
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_model}") <(echo "${reported_model}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
CPU Model: AMD EPYC Processor (with IBPB)                                CPU Model: AMD EPYC Processor (with IBPB)
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 09: Load Average [1 pts]

```

expected_load="Load Average (1/5/15 min): 1.87 1.27 1.07"
reported_load=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Load[[:space:]]*Average')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_load}") <(echo "${reported_load}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Load Average (1/5/15 min): 1.87 1.27 1.07                                Load Average (1/5/15 min): 1.87 1.27 1.07
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 10: Memory: 0%, 50% Usage [1 pts]

Checks the output and progress bar when memory usage is 0% and 50%.

```

cp "${TEST_DIR}/mem/zero" "${TEST_DIR}/fakeproc/meminfo"
reported_mem=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Memory[[:space:]]*Usage')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
cp "${TEST_DIR}/mem/original" "${TEST_DIR}/fakeproc/meminfo"
compare --ignore-all-space --ignore-case \
    <(echo "${expected_mem1}") <(echo "${reported_mem}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Memory Usage: [--------------------] 0.0% (0.0 GB / 500.0 GB)          | Memory Usage: [-------------------] 0.0% (0.0 GB / 0.0 GB)
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> FAIL
```

## Test 11: Memory: 94.9%, 100% Usage [1 pts]

Checks the output and progress bar when memory usage is 94.9% and 100%.

```

cp "${TEST_DIR}/mem/95" "${TEST_DIR}/fakeproc/meminfo"
reported_usage1=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'Memory[[:space:]]*Usage')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
cp "${TEST_DIR}/mem/original" "${TEST_DIR}/fakeproc/meminfo"
compare --ignore-all-space --ignore-case \
    <(echo "${expected_usage1}") <(echo "${reported_usage1}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Memory Usage: [###################-] 94.9% (474.5 GB / 500.0 GB)       | Memory Usage: [-------------------] 0.0% (474.0 GB / 500.0 GB)
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> FAIL
```

## Test 12: CPU Usage [1 pts]

CPU usage should register as 0% when both samples are identical

```

reported_cpu=$(./inspector -p "${TEST_DIR}/fakeproc" -r \
    | grep -i 'CPU[[:space:]]Usage')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: hardware 
compare --ignore-all-space --ignore-case \
    <(echo "${expected_cpu}") <(echo "${reported_cpu}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
CPU Usage:    [--------------------] 0.0%                                CPU Usage: [--------------------] 0.0% 
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 13: Task Count [1 pts]

```

reported_tasks=$(./inspector -p "${TEST_DIR}/fakeproc" -t \
    | grep -i 'Tasks[[:space:]]*running')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: task_list
compare --ignore-all-space --ignore-case \
    <(echo "${expected_tasks}") <(echo "${reported_tasks}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Tasks running: 1487                                                      Tasks running: 1487
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> OK

test_end
```

## Test 14: Correct Process Info, Name Truncation, and Sub-Task Count (9256) [2 pts]

Note that there are TWO possible correct outputs

```

reported_line=$(./inspector -p "${TEST_DIR}/fakeproc" -t | grep '88888')
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: task_list
compare --ignore-all-space \
    <(echo "${expected_line1}") <(echo "${reported_line}")

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
88888 |       zombie | really_really_long_named_ | alhanson |  9256    |
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> FAIL
result1=$?

compare --ignore-all-space \
    <(echo "${expected_line2}") <(echo "${reported_line}")

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
88888 |       zombie | really_really_long_named_ | root      |  9256   |
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> FAIL
result2=$?

if [[ $result1 -ne 0 && $result2 -ne 0 ]]; then
    test_end 1
fi
```

## Test 15: Task States [1 pts]

Checks for the various task states in the output

```

matches=$(./inspector -p "${TEST_DIR}/fakeproc" -t \
    | grep 'tracing stop' | wc -l)
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: task_list

list=$(./inspector -p "${TEST_DIR}/fakeproc" -t)
inspector.c:210:main(): Using alternative proc directory: /home/alhanson/projects/prj1/P1-alhanson7210/tests/fakeproc
inspector.c:233:main(): View options selected: task_list

# Now check for each process state (running, sleeping, disk sleep, zombie, dead)
[ ${matches} -eq 1 ] \
    && grep 'sleeping' <<< "${list}" &>/dev/null \
    && grep 'running' <<< "${list}" &>/dev/null \
    && grep 'zombie' <<< "${list}" &>/dev/null
test_end
```

## Test 16: Memory Leak Check [1 pts]

```

valgrind ./inspector -p "${TEST_DIR}/fakeproc" 2>&1 \
    | grep 'no leaks are possible'
==12166== All heap blocks were freed -- no leaks are possible

test_end
```

## Test 17: Documentation Check [1 pts]

Ensures documentation is provided for each function and data structure

```

if ! ( which doxygen &> /dev/null ); then
    # "Doxygen is not installed. Please install (as root) with:"
    # "pacman -Sy doxygen"
    test_end 1
fi

doxygen "${TEST_DIR}/Doxyfile" 2>&1 \
    | grep -v '(variable)' \
    | grep -v '(macro definition)' \
    | grep 'is not documented' \
        && test_end 1

test_end 0
```

## Test 18: Static Analysis [1 pts]

Checks for programming and stylistic errors with cppcheck

```

if ! ( which cppcheck &> /dev/null ); then
    # "cppcheck is not installed. Please install (as root) with:"
    # "pacman -Sy cppcheck"
    test_end 1
fi

cppcheck --enable=warning,style,performance,portability,unusedFunction \
    --error-exitcode=1 \
    "${TEST_DIR}/../"
Checking /home/alhanson/projects/prj1/P1-alhanson7210/inspector.c ...
1/2 files checked 82% done
Checking /home/alhanson/projects/prj1/P1-alhanson7210/tests/virtualproc/virtualproc.c ...
2/2 files checked 100% done

test_end
```

## Test 19: Compiler warnings check [1 pts]

To pass, eliminate all compiler warnings/errors

```

gcc -Wall -Werror -lm "${TEST_DIR}"/../*.c

test_end
```

## Test 20: Live View [2 pts]

```

output=$(FAKE_MEMINFO="${TEST_DIR}/virtualproc/meminfo" \
FAKE_LOADAVG="${TEST_DIR}/virtualproc/loadavg" \
FAKE_STAT="${TEST_DIR}/virtualproc/stat" \
LD_PRELOAD="${TEST_DIR}/virtualproc/virtualproc.so" \
    timeout 1 ./inspector -l 2> /dev/null)

reported_loads=$(echo "${output}" | \
    grep -io 'Load[[:space:]]*Average[[:space:]]*(1/5/15 min):[[:space:]]*\([0-9.]*[0-9]\)[[:space:]]*\([0-9.]*[0-9]\)[[:space:]]*\([0-9.]*[0-9]\)' | sort | uniq)
compare --ignore-all-space --ignore-case \
    <(echo "${expected_loads}") <(echo "${reported_loads}") || test_end 1

Expected Program Output                                                | Actual Program Output
-----------------------------------------------------------------------V----------------------------------------------------------------------
Load Average (1/5/15 min): 0.99 0.99 0.99                                Load Average (1/5/15 min): 0.99 0.99 0.99
Load Average (1/5/15 min): 1.00 1.00 1.11                              <
Load Average (1/5/15 min): 78.10 40.40 40.40                           <
Load Average (1/5/15 min): 99.99 99.99 99.99                           <
-----------------------------------------------------------------------^----------------------------------------------------------------------
 --> FAIL
```

