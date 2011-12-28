#################################################################
# This file is part of glyr
# + a commnandline tool and library to download various sort of musicrelated metadata.
# + Copyright (C) [2011-2012]  [Christopher Pahl]
# + Hosted at: https://github.com/sahib/glyr
#
# glyr is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# glyr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with glyr. If not, see <http://www.gnu.org/licenses/>.
#################################################################
#!/usr/bin/ruby

SCRIPT_PATH = File.absolute_path(File.dirname(__FILE__))

system("rm -rf /tmp/check && mkdir -p /tmp/check")

def run_c_test name
    system("echo 'G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --leak-check=full #{SCRIPT_PATH}/../bin/#{name}' | sh")
end

run_c_test "check_api"
run_c_test "check_opt"
run_c_test "check_dbc"
