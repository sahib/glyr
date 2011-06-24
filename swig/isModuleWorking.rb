begin
  if ARGV.size > 0
    puts "- Testing module '#{ARGV[0]}'"
    require ARGV[0]
    puts "- Module '#{ARGV[0]}' seems to work."
  else
    puts "Usage: isModuleWorking.rb <modulename>" 
  end
rescue Exception => e
  puts "- Error: #{e}"
end
