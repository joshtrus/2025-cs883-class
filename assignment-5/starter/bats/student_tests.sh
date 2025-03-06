#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Pipeline: ls | grep \".c\"" {
  run ./dsh <<EOF
ls | grep ".c"
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"dsh_cli.c"* ]] || [[ "$output" == *"dshlib.c"* ]]
}

@test "Exit command prints exiting message" {
  run ./dsh <<EOF
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"exiting..."* ]]
  [[ "$output" == *"cmd loop returned 0"* ]]
}

@test "Output redirection > creates file with expected content" {
  run ./dsh <<EOF
echo "hello, class" > out.txt
cat out.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello, class"* ]]
}

@test "Output append redirection >> appends text correctly" {
  run ./dsh <<EOF
echo "hello, class" > out.txt
echo "this is line 2" >> out.txt
cat out.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello, class"* ]]
  [[ "$output" == *"this is line 2"* ]]
}

@test "cd command changes working directory" {
  run ./dsh <<EOF
pwd
cd ..
pwd
exit
EOF
  [ "$status" -eq 0 ]
  first=$(echo "$output" | sed -n '1p' | tr -d '[:space:]')
  third=$(echo "$output" | sed -n '3p' | tr -d '[:space:]')
  [ "$first" != "$third" ]
}

@test "rc returns error code after non-existent command" {
  run ./dsh <<EOF
not_a_command
rc
exit
EOF
  echo "Captured output:"
  echo "$output"
  [ "$status" -eq 0 ]
  # Check that the output contains one of the expected error messages.
  [[ "$output" == *"No such file or directory"* ]] || [[ "$output" == *"failed to execute command"* ]]
  # Check that the rc builtin printed a non-zero error code (one or more digits)
  [[ "$output" =~ [1-9][0-9]* ]]
}

@test "Empty input prints warning" {
  run ./dsh <<EOF
       
exit
EOF
  [[ "$output" == *"warning: no commands provided"* ]]
  [ "$status" -eq 0 ]
}

@test "Quoted arguments preserve spaces" {
  run ./dsh <<EOF
echo "   hello   world  "
exit
EOF
  [[ "$output" == *"   hello   world  "* ]]
  [ "$status" -eq 0 ]
}

@test "Pipeline with three commands" {
  run ./dsh <<EOF
echo abc | tr a-z A-Z | rev
exit
EOF
  [[ "$output" =~ "CBA" ]]
  [ "$status" -eq 0 ]
}

@test "Input redirection: cat < file" {
  run bash -c 'echo "line1" > temp.txt'
  [ "$status" -eq 0 ]
  run ./dsh <<EOF
cat < temp.txt
exit
EOF
  [[ "$output" == *"line1"* ]]
  [ "$status" -eq 0 ]
  run bash -c 'rm temp.txt'
}


@test "Multiple spaces between arguments" {
  run ./dsh <<EOF
echo    spaced    out   text
exit
EOF
  [[ "$output" == *"spaced out text"* ]]
  [ "$status" -eq 0 ]
}
