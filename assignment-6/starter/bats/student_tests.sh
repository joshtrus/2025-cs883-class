#!/usr/bin/env bats

# File: student_tests.sh
# Extended tests for remote shell project without teardown

setup() {
  # Start the server in the background on port 7890.
  ./dsh -s -p 7890 &
  server_pid=$!
  sleep 1
}

@test "Simple command execution: ls" {
  run ./dsh -c -p 7890 <<EOF
ls
exit
EOF
  [ "$status" -eq 0 ]
  [ -n "$output" ]
}

@test "Pipeline execution: ls | grep \".c\"" {
  run ./dsh -c -p 7890 <<EOF
ls | grep ".c"
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ ".c" ]]
}

@test "Output redirection (overwrite): echo > file" {
  run ./dsh -c -p 7890 <<EOF
echo "hello, class" > testfile.txt
cat testfile.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "hello, class" ]]
}

@test "Output redirection (append): echo >> file" {
  run ./dsh -c -p 7890 <<EOF
echo "first line" > append.txt
echo "second line" >> append.txt
cat append.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "first line" ]]
  [[ "$output" =~ "second line" ]]
}

@test "Input redirection: cat < file" {
  run ./dsh -c -p 7890 <<EOF
echo "input test" > infile.txt
cat < infile.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "input test" ]]
}

@test "Complex pipeline with redirection: echo | tr | sort > file" {
  run ./dsh -c -p 7890 <<EOF
echo "delta alpha gamma beta" | tr " " "\n" | sort > pipeline.txt
cat pipeline.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "alpha" ]]
  [[ "$output" =~ "beta" ]]
  [[ "$output" =~ "delta" ]]
  [[ "$output" =~ "gamma" ]]
}

@test "Empty command produces warning" {
  run ./dsh -c -p 7890 <<EOF
  
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "warning:" ]]
}

@test "Too many piped commands error" {
  run ./dsh -c -p 7890 <<EOF
cmd1 | cmd2 | cmd3 | cmd4 | cmd5 | cmd6 | cmd7 | cmd8 | cmd9
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "error: piping limited to 8 commands" ]]
}

@test "Redirection permission error: echo to /root" {
  run ./dsh -c -p 7890 <<EOF
echo "hello" > /root/out.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "Permission denied" ]]
}

@test "Built-in command: cd without argument" {
  run ./dsh -c -p 7890 <<EOF
cd
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "cd error: missing argument" ]]
}

@test "Built-in command: cd with invalid directory" {
  run ./dsh -c -p 7890 <<EOF
cd /nonexistentdirectory
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "cd error:" ]]
  [[ "$output" =~ "No such file" || "$output" =~ "not found" ]]
}


@test "Quoted arguments handling" {
  run ./dsh -c -p 7890 <<EOF
echo "this is a test"
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "this is a test" ]]
}

@test "Stop-server command shuts down server" {
  run ./dsh -c -p 7890 <<EOF
stop-server
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" =~ "cmd loop returned 0" ]]
}
