/*
 * simple test for curl.
 *
 * It's hard to test the curl plugin since it depends on a lot of things.
 *
 * This test depends on http://flusspferd.org/foo.txt containing the line foo and
 * a working connection.
 */

const cURL = require('curl');

var c = new cURL.Easy();
c.options.url = 'http://flusspferd.org/foo.txt';
c.perform();
