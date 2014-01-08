# encoding: UTF-8
require 'open-uri'
require 'nokogiri'

def read_url(html_file)
  doc_content = Nokogiri::HTML(File.read(html_file))
  print "Now parsing #{html_file}... "
  file_name = "#{File.basename(html_file, ".html")}.xml"
  Dir.mkdir("parsed_docs") unless File.exists? "parsed_docs"
  File.open("parsed_docs/#{file_name}", "w+") do |f|
    # Gets all the text between tags from the page
    # and writes it to the file.
    plain_text = doc_content.search('//text()')
    f.write plain_text
  end
  print "#{file_name} created!\n"
end

File.open("links.txt", "r") do |f|
  while line = f.gets
      # read_url(line) unless ["", "\n"].include? line
  end
end

def traverse(root_dir)
  Dir.foreach(".") do |file|
    puts file
    traverse(file) if File.directory? file
    puts File.expand_path file
    read_url(File.expand_path file) if File.extname(file) =~ /\.htm/
  end
end

traverse(ARGV[0]) unless ARGV.length == 0
