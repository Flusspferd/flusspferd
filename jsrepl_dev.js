// Set default paths to the plugin dirs
Importer.defaultPaths = ['js/src', 'build/default/plugins/sqlite3', 
  'build/default/plugins/curl', 'build/default/plugins/environment',
  'build/default/src'];

Importer.preload['xml'] = function() { return this.$importer.load('flusspferd-xml'); }
Importer.preload['io'] = function() { return this.$importer.load('flusspferd-io'); }

prelude = 'prelude.js';
