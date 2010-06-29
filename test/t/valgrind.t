#vi:filetype=perl


use lib 'lib';
use Test::Nginx::Socket skip_all => 'not inplemented yet';

plan tests => 2;

no_long_string();

run_tests();

__DATA__

=== TEST 1: basic
--- config
    location /foo {
        set_form_input $foo name;
    }
--- more_headers
Content-Type: application/x-www-form-urlencoded
--- request
POST /foo
name=calio&someting=something
--- response_body
calio
