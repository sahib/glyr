#!/usr/bin/ruby

SCRIPT_PATH = File.absolute_path(File.dirname(__FILE__))

system("rm -rf /tmp/check && mkdir -p /tmp/check")

def run_c_test name
    system("echo 'G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full #{SCRIPT_PATH}/../bin/#{name}' | sh")
end

run_c_test "check_api"
run_c_test "check_opt"
run_c_test "check_dbc"
