require 'open-uri'
require 'nokogiri'

def read_url(url)
    # Opens the url passed from the file
    doc = Nokogiri::HTML(open(url))
    print "Now parsing #{url[0...-1]}... "
    domain = url.scan(/^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/~\w \.-]*)*\/?$/)[0][1..2]
    file_name = "#{domain[0]}.#{domain[1]}.txt"
    # Opens a file naming it after the url passed.
    File.open("pages/#{file_name}", "w+") do |f|
        # Gets all the text between tags from the page
        # and writes it to the file.
        doc.search('//text()').each {|str| f.write str}
    end
    print "#{file_name} created!\n"
end

File.open("links.txt", "r") do |f|
    while line = f.gets
        read_url(line) unless ["", "\n"].include? line
    end
end
