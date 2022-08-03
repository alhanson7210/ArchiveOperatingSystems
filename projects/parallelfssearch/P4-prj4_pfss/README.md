# Project 4: Parallel File System Search

See project description here: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-4.html

*Writer: Alex Hanson*

University of San Francisco

CS 326 Operating Systems

## About This Project:
_Implementing a Unix utility that recursively searches for matching words in text files_

## Program Output:
```
[ /home/alhanson/projects/prj4/P4-prj4_pfss/test-output.md | 192 | orw- | local nontest file system for matches 2 pts ]

Format:
[ /absolute/path/to/file | line-number | permissions | the line the word was found in ]
```

## Commit:
```
Best commit or most stable -> commit d6420b30b7de5e0800357f20407dfd0aaf3fba25
```

## Note:
```
The versions before commit d6420b3 don't have ./prep issues from trying to fix whitespace
Below is also the test case before this current version thats after 12(to see if I can fix whitespace)

I got this from trying to solve the issue(stripping) a different way:

[ /home/alhanson/path/projects/prj4/P4-prj4_pfss/tests/test-fs/xyz/magic/magical-unicorn.c | 1 | or-- | floccinaucinihilipilification ]

This didn't pass the test case, so I submitted the code for what I did to get this output(line above ^) to show for as proof

If I get that point, that's great, but if not, project 4 should be 12/15 pts based off of previous versions before 12pm

Test Output is provided as well
```

## Test Output
```
./tests/run_tests
Running Tests: (v8)
 * 01 Arguments            [1 pts]  [  OK  ]
 * 02 Invalid Thread Count [1 pts]  [  OK  ]
 * 03 Invalid Directory    [1 pts]  [  OK  ]
 * 04 Basic Search         [1 pts]  [  OK  ]
 * 05 Many Terms           [1 pts]  [  OK  ]
 * 06 Whitespace           [1 pts]  [ FAIL ]
 * 07 Thread Performance   [1 pts]  [  OK  ]
 * 08 Exact Match          [1 pts]  [  OK  ]
 * 09 Local Filesystem     [2 pts]  [  OK  ]
 * 10 Thread Count         [2 pts]  [ FAIL ]
 * 11 Static Analysis      [1 pts]  [  OK  ]
 * 12 Documentation        [1 pts]  [  OK  ]
 * 13 Leak Check           [1 pts]  [  OK  ]
Execution complete. [12/15 pts] (80.0%)
```
