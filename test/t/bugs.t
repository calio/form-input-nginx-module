# vi:filetype=perl

use lib 'lib';
use Test::Nginx::Socket;

#repeat_each(3);

plan tests => repeat_each() * 2 * blocks();

no_long_string();

run_tests();

__DATA__

=== TEST 1: charset postfix
--- config
	location /bar2 {
		set_form_input $foo bar;
		echo $foo;
	}
--- more_headers
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
--- request
POST /bar2
bar=32
--- response_body
32
--- timeout: 3



=== TEST 2: test case sensitivity
--- config
	location /bar2 {
		set_form_input $foo bar;
		echo $foo;
	}
--- more_headers
Content-Type: application/x-www-Form-UrlencodeD
--- request
POST /bar2
bar=32
--- response_body
32
--- timeout: 3

