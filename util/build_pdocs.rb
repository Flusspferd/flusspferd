#!/usr/bin/env ruby

require 'rubygems'
require 'rake'



module HippoDocsHelper
  ROOT_DIR      = File.join(File.expand_path(File.dirname(__FILE__)), '..')
  DOC_DIR       = File.join(ROOT_DIR, 'build', 'html', 'js')
  TEMPLATE_DIR  = File.join(ROOT_DIR, 'vendor', 'pdoc-template', 'html')


  def self.require_pdoc
    require_submodule('PDoc', 'pdoc')
  end
  

  def self.build_docs()

    rm_rf DOC_DIR
    begin
      files = Dir.glob( File.join(ROOT_DIR, "src", "**","*.pdoc") )
      files << Dir.glob( File.join(ROOT_DIR, "src","**","*.js") )
      files.flatten!

      if files.empty?
        puts "No input pdoc files!"
        exit 1;
      end
      files << {
        :output => DOC_DIR,
        :templates => TEMPLATE_DIR,
        :syntax_highlighter => :none,
        #:index_page => 'README.markdown'
      }
      PDoc::Runner.new(*files).run
      exit 0;
    rescue PDoc::ParseError => e
      puts e.message
    rescue ArgumentError => e
      puts "ArgError: " << e
    end
    exit 1;
  end


  def self.require_submodule(name, path)
    begin
      require path
    rescue LoadError => e
      missing_file = e.message.sub('no such file to load -- ', '')
      if missing_file == path
        puts "\nIt looks like you're missing #{name}. Just run:\n\n"
        puts "  $ git submodule init"
        puts "  $ git submodule update vendor/#{path}"
        puts "\nand you should be all set.\n\n"
      else
        puts "\nIt looks like #{name} is missing the '#{missing_file}' gem. Just run:\n\n"
        puts "  $ gem install #{missing_file}"
        puts "\nand you should be all set.\n\n"
      end
      exit
    end
  end
end

%w[pdoc].each do |name|
  $:.unshift File.join(HippoDocsHelper::ROOT_DIR, 'vendor', name, 'lib')
end

if __FILE__ == $0
  HippoDocsHelper.require_pdoc
  HippoDocsHelper.build_docs
end
