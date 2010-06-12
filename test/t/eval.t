# vi:filetype=

use lib 'lib';
use Test::Nginx::Socket;

#repeat_each(3);

plan tests => repeat_each() * 2 * blocks();

no_long_string();

run_tests();

#no_diff();

__DATA__

=== TEST 1: eval + form_input
--- config
    location /bar {
        set_form_input $dummy;
        eval_subrequest_in_memory off;
        eval_override_content_type text/plain;
        eval_buffer_size 1k;
        eval $res {
            default_type text/plain;
            set_form_input $foo bar;
            echo 'hi'; #$foo;
        }
        echo [$res];
    }
--- more_headers
Content-Type: application/x-www-form-urlencoded
--- request
POST /bar
bar=3
--- response_body
[3]

