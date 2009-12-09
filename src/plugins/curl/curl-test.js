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
c.options.userAgent = 'Flussperd!';
c.options.cookie = 'flusspferd=cool';
//c.options.interface = 'eth0';
//c.options.localport = 8234;
/*
// ssh -NfD 8080 <somehost>
c.options.proxy = 'localhost';
c.options.proxyport = 8080;
c.options.proxytype = cURL.PROXY_SOCKS5;
c.options.noproxy = 'flusspferd.org';
*/
c.options.port = 80;
c.options.url = 'http://flusspferd.org/foo.txt';
c.options.writefunction = function(data,size) { // size is element width not data.length! confusing? :-D
	if(data.decodeToString() === 'foo\n') {
		print("Ok");
	}
	else {
		print("Got: ");
		print(data.decodeToString());
	}
	return data.length; // return sth. else if error
};
c.options.noprogress = 0; // set to 0 or else progressFunction won't get called!
c.options.progressfunction = function(dltotal, dlnow, ultotal, ulnow) {
	if(dltotal != 0) {
		print("loading... " + dlnow/dltotal*100 + "%");
	}
	return 0; // return 0 if ok. else abort!
};
c.perform();
