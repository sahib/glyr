#!/usr/bin/ruby

SCRIPT_PATH = File.absolute_path(File.dirname(__FILE__))

system("echo 'G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full #{SCRIPT_PATH}/../bin/check_api' | sh")
