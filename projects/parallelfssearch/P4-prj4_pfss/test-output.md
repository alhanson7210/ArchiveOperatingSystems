## Test 01: No Arguments [1 pts]

If no arguments are provided, print usage information and quit. If -h is

```

reference_run ./prep -h

run ./prep

compare_outputs || test_end

Expected Program Output                                                       | Actual Program Output
------------------------------------------------------------------------------V------------------------------------------------------------------------------
Usage: ./prep [-eh] [-d directory] [-t threads] search_term1 search_term2 ...    Usage: ./prep [-eh] [-d directory] [-t threads] search_term1 search_term2 ...

Options:                                                                         Options:
    * -d directory    specify start directory (default: CWD)                         * -d directory    specify start directory (default: CWD)
    * -e              print exact name matches only                                  * -e              print exact name matches only
    * -h              show usage information                                         * -h              show usage information
    * -t threads      set maximum threads (default: # procs)                         * -t threads      set maximum threads (default: # procs)
------------------------------------------------------------------------------^------------------------------------------------------------------------------
 --> OK
```

## Test 02: Invalid Thread Count [1 pts]

Checks to make sure an invalid thread count is handled properly.

```

run ./prep -t -1 wow

run ./prep -t hello world

test_end
```

## Test 03: Invalid Directory [1 pts]

The program should not crash when given an invalid directory 

```

run ./prep -t 1000 -e -d /this/does/not/exist xxx yyy zzz
opendir: No such file or directory

test_end
```

## Test 04: Basic Searches [1 pts]

```

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs/xyz" sea

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs/xyz" sea blerpoblagoperatooogazoa

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 05: Many-Term Search [1 pts]

```

reference_run "${TEST_DIR}/prep.sh" -t 2 -e -d "${TEST_DIR}/test-fs" \
    Cochrone aquamarine Callistonian chandeliers encephalography encyclopedic institutionalized \
        | sort
Searching for Cochrone in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for aquamarine in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for Callistonian in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for chandeliers in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for encephalography in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for encyclopedic in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for institutionalized in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs

run "${TEST_DIR}/../prep" -t 2 -e -d "${TEST_DIR}/test-fs" \
    Cochrone aquamarine Callistonian chandeliers encephalography encyclopedic institutionalized \
        | sort

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 06: Whitespace Handling [1 pts]

```

reference_run "${TEST_DIR}/prep.sh" -d "${TEST_DIR}/test-fs" \
    blerpoblagoperatooogazoa floccinaucinihilipilification
Searching for blerpoblagoperatooogazoa in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs
Searching for floccinaucinihilipilification in /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" \
    blerpoblagoperatooogazoa floccinaucinihilipilification

compare_outputs

-- diff of outputs shown below --
1c1
< [ /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs/xyz/magic/magical-unicorn.c | 1 | or-- | floccinaucinihilipilification ]
---
> 
---------------------------------
 --> FAIL

test_end
 --> Test failed (255)
```

## Test 07: Thread Performance [1 pts]

Tests the performance improvement (>= 1.2 in at least 1 iteration of 3)

```

for i in {1..3}; do
    run "${TEST_DIR}/../prep" -t 1 -d "${TEST_DIR}/test-fs" the
    run1="${program_runtime}"

    run "${TEST_DIR}/../prep" -t 2 -d "${TEST_DIR}/test-fs" the
    run2="${program_runtime}"

    awk '
    {
        improvement = ($1 / $2)
        printf "Speed improvement: %.2f\n", improvement
        printf "(Must be >= 1.2)\n"
        if (improvement < 1.2) {
            exit 1
        } else {
            exit 0
        }
    }' <<< "${run1} ${run2}"
    
    if [[ $? -eq 0 ]]; then
        test_end 0
    fi
done
Speed improvement: 1.00
(Must be >= 1.2)
Speed improvement: 1.00
(Must be >= 1.2)
Speed improvement: 1.33
(Must be >= 1.2)
```

## Test 08: Exact Match [1 pts]

```

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -e \
    HELLO WHATS COOKIN

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

run "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -e HeLLo

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 09: Searches the local (non-test) file system for matches. [2 pts]

```

reference_run "${TEST_DIR}/prep.sh" -d /usr/share hippopotamus | sort
Searching for hippopotamus in /usr/share
grep: /usr/share/factory/etc/crypttab: Permission denied
grep: /usr/share/factory/etc/gshadow: Permission denied
grep: /usr/share/factory/etc/shadow: Permission denied
grep: /usr/share/polkit-1/rules.d: Permission denied

run "${TEST_DIR}/../prep" -d /usr/share hippopotamus | sort

compare_outputs || test_end

-- diff of outputs shown below --
---------------------------------
 --> OK

reference_run "${TEST_DIR}/prep.sh" -d /usr/share/nano color | sort
Searching for color in /usr/share/nano

run "${TEST_DIR}/../prep" -d /usr/share/nano color | sort
prep: malloc.c:2389: sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) || ((unsigned long) (old_size) >= MINSIZE && prev_inuse (old_top) && ((unsigned long) old_end & (pagesize - 1)) == 0)' failed.
timeout: the monitored command dumped core

```
 --> Test failed (134)

compare_outputs

-- diff of outputs shown below --
---------------------------------
 --> OK

test_end
```

## Test 10: Ensures the number of threads is limited properly [2 pts]

```

for i in {1..5}; do
    threads=$(( i + 10 ))
    "${TEST_DIR}/../prep" -d / -t "${threads}" \
        this is a very large test of the project a \
        long time ago in a galaxy far far away &> /dev/null &
    job=${!}

    sleep 1
    detected_threads=$(grep Threads /proc/${job}/status | awk '{print $2}')
    echo "Number of active threads detected: ${detected_threads}"
    (( detected_threads-- )) # To account for the main thread
    kill -9 "${job}"

    if [[ ${threads} -eq ${detected_threads} ]]; then
        test_end 0
    fi
done
grep: /proc/30194/status: No such file or directory
Number of active threads detected: 
./tests/10-Thread-Count-2.sh: line 16: kill: (30194) - No such process
grep: /proc/30208/status: No such file or directory
Number of active threads detected: 
./tests/10-Thread-Count-2.sh: line 16: kill: (30208) - No such process
grep: /proc/30217/status: No such file or directory
Number of active threads detected: 
./tests/10-Thread-Count-2.sh: line 16: kill: (30217) - No such process
grep: /proc/30226/status: No such file or directory
Number of active threads detected: 
./tests/10-Thread-Count-2.sh: line 16: kill: (30226) - No such process
grep: /proc/30235/status: No such file or directory
Number of active threads detected: 
./tests/10-Thread-Count-2.sh: line 16: kill: (30235) - No such process

test_end 1
 --> Test failed (1)
```

## Test 11: Static Analysis [1 pts]

Checks for programming and stylistic errors with cppcheck and gcc/clang

```

if ! ( which cppcheck &> /dev/null ); then
    # "cppcheck is not installed. Please install (as root) with:"
    # "pacman -Sy cppcheck"
    test_end 1
fi

cppcheck --enable=warning,performance \
    --error-exitcode=1 \
    "${TEST_DIR}/../prep.c" || test_end 1
Checking /home/alhanson/projects/prj4/P4-prj4_pfss/prep.c ...

cc -Wall -Werror -pthread "${TEST_DIR}"/../{*.c,*.h} || test_end 1

test_end
```

## Test 12: Documentation Check [1 pts]

Ensures documentation is provided for each function and data structure

```

if ! ( which doxygen &> /dev/null ); then
    # "Doxygen is not installed. Please install (as root) with:"
    # "pacman -Sy doxygen"
    test_end 1
fi

# All .c and .h files will be considered; if you'd like to exclude temporary or
# backup files then add a different extension (such as .bak).
for file in $(find . -type f \( -iname "*.c" -o -iname "*.h" \) -not -path "./tests/*"); do
    if ! ( grep '@file' "${file}" &> /dev/null ); then
        echo "@file documentation preamble not found in ${file}"
        file_failed=true
    fi
done

if [[ ${file_failed} == true ]]; then
    # A file didn't have the @file preamble
    test_end 1
fi

doxygen "${TEST_DIR}/Doxyfile" 2>&1 \
    | grep -v '(variable)' \
    | grep -v '(macro definition)' \
    | grep 'is not documented' \
        && test_end 1

test_end 0
```

## Test 13: Memory Leak Check [1 pts]

```

valgrind --leak-check=full \
    "${TEST_DIR}/../prep" -d "${TEST_DIR}/test-fs" -t 50 \
        this is only a test \
    | grep 'are definitely lost'
==30269== Memcheck, a memory error detector
==30269== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==30269== Using Valgrind-3.14.0 and LibVEX; rerun with -h for copyright info
==30269== Command: /home/alhanson/projects/prj4/P4-prj4_pfss/tests/../prep -d /home/alhanson/projects/prj4/P4-prj4_pfss/tests/test-fs -t 50 this is only a test
==30269== 
==30269== 
==30269== HEAP SUMMARY:
==30269==     in use at exit: 0 bytes in 0 blocks
==30269==   total heap usage: 12 allocs, 12 frees, 231,388 bytes allocated
==30269== 
==30269== All heap blocks were freed -- no leaks are possible
==30269== 
==30269== For counts of detected and suppressed errors, rerun with: -v
==30269== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
[[ $? -eq 1 ]]

test_end
```

