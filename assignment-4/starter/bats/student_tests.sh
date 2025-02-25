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

@test "Echo prints correctly" {
    run ./dsh <<EOF
echo "hello, world"
exit
EOF
    [[ "$output" == *"hello, world"* ]]
    [ "$status" -eq 0 ]
}

@test "Empty command prints warning" {
    run ./dsh <<EOF

exit
EOF
    [[ "$output" == *"warning: no commands provided"* ]]
    [ "$status" -eq 0 ]
}


@test "cd with no argument does not change directory" {
    current=$(pwd);
    run ./dsh <<EOF
    cd

EOF
    output=$(pwd);
    [ "$status" -eq 0 ]
    [ "$output" = "$current" ]
}

@test "cd with valid directory changes working directory" {
    run ./dsh <<EOF
pwd
cd ..
pwd
exit
EOF
    first_pwd=$(echo "$output" | sed -n '1p' | tr -d '[:space:]')
    second_pwd=$(echo "$output" | sed -n '3p' | tr -d '[:space:]')
    [ "$first_pwd" != "$second_pwd" ]
    [ "$status" -eq 0 ]
}

@test "echo with quoted text preserves inner spaces" {
    run ./dsh <<EOF
echo "   hello,   world  "
exit
EOF

    [[ "$output" == *"hello,   world"* ]]
    [ "$status" -eq 0 ]
}

@test "ls -l runs without errors" {
    run ./dsh <<EOF
ls -l
exit
EOF
    [[ "$output" == *"total"* ]] || true
    [ "$status" -eq 0 ]
}

@test "cd with invalid directory prints error" {
    run ./dsh <<EOF
cd /nonexistent_directory
rc
exit
EOF
    [[ "$output" == *"No such file or directory"* ]] || [[ "$output" == *"cd:"* ]]
    [ "$status" -eq 0 ]
}

@test "Command with extra spaces between arguments" {
    run ./dsh <<EOF
echo      spaced    out   arguments
exit
EOF
    [[ "$output" == *"spaced out arguments"* ]]
    [ "$status" -eq 0 ]
}
