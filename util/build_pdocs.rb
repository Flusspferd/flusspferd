#!/usr/bin/env ruby



module HippoDocsHelper
  ROOT_DIR      = File.join(File.expand_path(File.dirname(__FILE__)), '..')
  DOC_DIR       = File.join(ROOT_DIR, 'build', 'html', 'js')
  TEMPLATE_DIR  = File.join(ROOT_DIR, 'vendor', 'pdoc-template', 'html')


  def self.require_pdoc
    require_submodule('PDoc', 'pdoc')
    if !File.directory?(TEMPLATE_DIR)
      puts missing_submodule_msg("the pdoc-templates", "pdoc-template")
      exit
    end
  end
  

  def self.build_docs(extras)

    rm_rf DOC_DIR
    begin
      files = Dir.glob( File.join(ROOT_DIR, "libflusspferd", "**","*.pdoc") )
      files << Dir.glob( File.join(ROOT_DIR, "plugins","**","*.js") )
      files << Dir.glob( File.join(ROOT_DIR, "plugins","**","*.pdoc") )
      files << Dir.glob( File.join(ROOT_DIR, "js","**","*.js") )
      files << extras
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

  def self.require_gems()
    begin
      require 'rubygems'
      require 'rake'
    rescue LoadError => e
      missing_file = e.message.sub('no such file to load -- ', '')
      if missing_file == 'rubygems'
        puts "\nIt looks like rubygems is missing! Please install it."
      else
        puts "\nIt looks like #{name} is missing the '#{missing_file}' gem. Just run:\n\n"
        puts "  $ gem install #{missing_file}"
        puts "\nand you should be all set.\n\n"
      end
      exit
    end
  end

  class << self
    def missing_submodule_msg(name, path)
      "\nIt looks like you're missing #{name}. Just run:\n\n" +
      "  $ git submodule init\n" +
      "  $ git submodule update vendor/#{path}\n" +
      "\nand you should be all set.\n\n"
    end
  end

  def self.require_submodule(name, path)
    begin
      require path
    rescue LoadError => e
      missing_file = e.message.sub('no such file to load -- ', '')
      if missing_file == path
        puts missing_submodule_msg(name, path)
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
  HippoDocsHelper.require_gems
  HippoDocsHelper.require_pdoc
  HippoDocsHelper.build_docs(ARGV)
end
