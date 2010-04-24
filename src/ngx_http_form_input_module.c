#define DDEBUG 0
#include <ndk.h>
#include "ddebug.h"
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_flag_t    flag;
} ngx_http_form_input_loc_conf_t;


typedef struct {
    ngx_int_t         done;
    ngx_array_t        *params;
} ngx_http_form_input_ctx_t;


static ngx_int_t
ngx_http_set_form_input(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v);

static char *
ngx_http_set_form_input_conf_handler(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void *
ngx_http_form_input_create_loc_conf(ngx_conf_t *cf);

static char *
ngx_http_form_input_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t
ngx_http_form_input_init(ngx_conf_t *cf);

static ngx_int_t
ngx_http_form_input_handler(ngx_http_request_t *r);

static void ngx_http_form_input_post_read(ngx_http_request_t *r);


static ngx_command_t ngx_http_form_input_commands[] = {

    { ngx_string("set_form_input"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
      ngx_http_set_form_input_conf_handler,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_form_input_loc_conf_t, flag),
      NULL },

      ngx_null_command
};


static ngx_http_module_t ngx_http_form_input_module_ctx = {
    NULL,
    ngx_http_form_input_init,

    NULL,
    NULL,

    NULL,
    NULL,

    ngx_http_form_input_create_loc_conf,
    ngx_http_form_input_merge_loc_conf
};


ngx_module_t ngx_http_form_input_module = {
    NGX_MODULE_V1,
    &ngx_http_form_input_module_ctx,
    ngx_http_form_input_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_set_form_input(ngx_http_request_t *r, ngx_str_t *res,
    ngx_http_variable_value_t *v)
{
    ngx_http_form_input_loc_conf_t     *lcf;
    ngx_http_form_input_ctx_t            *ctx;
    ngx_chain_t                          *chain;
    ngx_int_t                            len, n;
    u_char                               *buf, *p, *last;

    dd("ndk handler:set form input handler");
    /*fprintf(stderr, "content-type:%s\n", r->headers_in.content_type->
        value.data); */

    dd("set default return value");
    res->data = (u_char *)"";
    res->len = 0;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_form_input_module);

    if(lcf == NULL) {
        dd("ndk:lcf is null");
        return NGX_OK;
    }

    if(lcf->flag == 0 || lcf->flag == NGX_CONF_UNSET) {
        dd("ndk:flag is 0");
        return NGX_OK;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);

    if (ctx == NULL) {
        dd("ndk handler:null ctx");
        return NGX_OK;
    }

    if (ctx->done != NGX_DONE && ctx->done != NGX_OK) {
        dd("ctx not done");
        return NGX_AGAIN;
    }

    chain = r->request_body->bufs;

    for (len = 0; chain != NULL; chain = chain->next) {
        len += (ngx_int_t)(chain->buf->last - chain->buf->start);
    }
    dd("len=%d", (int)len);
    if (len == 0) {
        res->len = 0;
        res->data = (u_char*)"";
        dd("post body len 0");
        return NGX_OK;
    }

    buf = ngx_palloc(r->pool, len);
    p = buf;
    chain = r->request_body->bufs;

    for (; chain != NULL; chain = chain->next) {
        n = (ngx_int_t)(chain->buf->last - chain->buf->start);
        ngx_memcpy(p, chain->buf->start, n);
        p += n;
    }

    /* fork from ngx_http_arg */
    p = buf;
    last = p + len;

    dd("buf=%p", buf);
    dd("last=%p", last);
    for ( /* void */; p < last; p++) {
        /* we need '=' after name, so drop one char from last */

        p = ngx_strlcasestrn(p, last - 1, v->data, v->len - 1);
        dd("%p", p);
        if (p == NULL) {
            res->data = (u_char*)"";
            res->len = 0;
            return NGX_OK;
        }

        if ((p == buf || *(p - 1) == '&') && *(p + v->len) == '=') {
            res->data = p + v->len + 1;

            p = ngx_strlchr(p, last, '&');

            if (p == NULL) {
                p = last;
            }

            res->len = p - res->data;

            return NGX_OK;
        }
    }
    res->len = 0;
    res->data = (u_char*)"";
    return NGX_OK;
}


static char *
ngx_http_set_form_input_conf_handler(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf)
{
    ndk_set_var_t                        filter;
    ngx_str_t                              *var_name, s;
    ngx_http_form_input_loc_conf_t         *filcf;
    u_char                                *p;
    dd("set form input conf handler");

    filcf = conf;
    if (filcf == NULL) {
        dd("filcf is null");
        return NGX_CONF_OK;
    }
    dd("filcf->flag is 1");
    filcf->flag = 1;

    filter.type = NDK_SET_VAR_MULTI_VALUE;
    filter.func = ngx_http_set_form_input;
    filter.size = 1;

    var_name = cf->args->elts;
    var_name++;
    if (cf->args->nelts == 2)
    {
        dd("if starts");
        p = var_name->data;
        p++;
        s.len = var_name->len - 1;
        s.data = p;
        dd("if ends");
    } else if (cf->args->nelts == 3) {
        dd("else starts");
        s.len = (var_name + 1)->len;
        s.data = (var_name + 1)->data;
        dd("else ends");
    }

    return ndk_set_var_multi_value_core (cf, var_name,  &s,
        &filter);
}


static void *
ngx_http_form_input_create_loc_conf(ngx_conf_t *cf)
{
    dd("form input create loc conf");
    ngx_http_form_input_loc_conf_t *conf;
    conf = ngx_palloc(cf->pool, sizeof(ngx_http_form_input_loc_conf_t));

    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->flag = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_form_input_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    dd("form input merge loc conf");
    ngx_http_form_input_loc_conf_t *prev = parent;
    ngx_http_form_input_loc_conf_t *conf = child;

    ngx_conf_merge_value(prev->flag, conf->flag, 0);

    return NGX_CONF_OK;
}


/* register a new rewrite phase handler */
static ngx_int_t
ngx_http_form_input_init(ngx_conf_t *cf)
{
    dd("post conf: form_input_init");

    ngx_http_handler_pt            *h;
    ngx_http_core_main_conf_t    *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);

    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_form_input_handler;

    return NGX_OK;
}


static ngx_int_t
ngx_http_form_input_handler(ngx_http_request_t *r)
{
    dd("rewrite phase:form_input_handler");
    ngx_http_form_input_loc_conf_t  *lcf;
    ngx_http_form_input_ctx_t       *ctx;
    ngx_str_t                        value;
    ngx_int_t                        rc;

    lcf = ngx_http_get_module_loc_conf(r, ngx_http_form_input_module);
    if (lcf->flag == 0 || lcf->flag == NGX_CONF_UNSET) {
        dd("rewrite phase:lcf->flag==0");
        return NGX_DECLINED;
    }

    if (r->method != NGX_HTTP_POST)
    {
        lcf->flag = 0;
        return NGX_DECLINED;
    }

    if(r->headers_in.content_type == NULL || r->headers_in.content_type->
            value.data == NULL) {
        dd("content_type is %s", r->headers_in.content_type == NULL?"NULL":
                "NOT NULL");
        lcf->flag = 0;
        return NGX_DECLINED;
    }

    value = r->headers_in.content_type->value;

    /* just focus on x-www-form-urlencoded if (r-> */
    if (value.len < (sizeof("application/x-www-form-urlencoded") - 1) ||
            ngx_strncmp("application/x-www-form-urlencoded", value.data,
            value.len) != 0) {
        dd("not application/x-www-form-urlencoded");
        lcf->flag = 0;
        /* dd("content-type:%s", r->headers_in.content_type->value.data); */
        return NGX_DECLINED;
    }

    dd("content type is application/x-www-form-urlencoded");
    ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);

    if (ctx == NULL) {
        dd("create new ctx");
        ctx = ngx_palloc(r->pool, sizeof(ngx_http_form_input_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }
        ctx->done = NGX_BUSY;
        ngx_http_set_ctx(r, ctx, ngx_http_form_input_module);
    } else {
        if (ctx->done) {
            return NGX_DECLINED;
        }
        dd("already have ctx");
    }
    dd("start to read request_body");

    rc = ngx_http_read_client_request_body(r, ngx_http_form_input_post_read);

    if (rc == NGX_AGAIN) {
        return NGX_AGAIN;
    }

    return NGX_DECLINED;
}


static void ngx_http_form_input_post_read(ngx_http_request_t *r)
{
    ngx_http_request_body_t        *rb;
    ngx_http_form_input_ctx_t     *ctx;

    dd("ngx_http_post_read");
    rb = r->request_body;

    ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);

    if (ctx == NULL) {
        dd("post body read:NULL ctx");
        ctx->done = NGX_DONE;
        return;
    } else if (ctx->done == NGX_OK || ctx->done == NGX_DONE) {
        dd("post body read:finished");
        return;
    }
/*
    if (rb == NULL) {
        dd("rb is NULL");
        ctx->done = NGX_ERROR;
        return;
    }

    if (rb->buf == NULL || rb->bufs == NULL) {
        dd("rb->buf is NULL");
        ctx->done = NGX_DECLINED;
        return;
    }
*/
    dd("post read done");
    ctx->done = NGX_DONE;
    ngx_http_core_run_phases(r);
}


/*
static ngx_int_t ngx_http_form_input_process_body(ngx_http_request_t *r,
    ngx_http_form_input_ctx_t *ctx)
{
    dd("process body:parse post body");
    ngx_chain_t     *chain;

    chain = r->request_body->bufs;

    if (chain == NULL) {
        dd("chain == NULL");
        return NGX_ERROR;
    }
    dd("chain != NULL");


    start = chain->buf->start;
    for (; chain != NULL; chain = chain->next) {
        buf = chain->buf;
        for (; buf->pos != buf->last; buf->pos++) {
            ch = *(buf->pos);
            if (ch == '=') {
                end = buf->pos;
                param = ngx_array_push(ctx->params);
                param->name.len = (ngx_int_t)(end - start);
                ngx_memcpy(param->name.data, start, param->name.len);
                start = end;
            } else if (ch == '&') {
                end = buf->pos;
                param->value.len = (ngx_int_t)(end - start);
                ngx_memcpy(param->value.data, start, param->value.len);
                start = end;
            }
        }
    }
    dd("finished loop");
    end = buf->pos;
    param->value.len = (ngx_int_t)(end - start);
    ngx_memcpy(param->value.data, start, param->value.len);


    dd("process body:finished");
    return NGX_DONE;
}
*/


/*
static ngx_int_t ngx_http_form_input_header_filter(ngx_http_request_t *r)
{
    dd("input header filter");
    ngx_http_form_input_loc_conf_t *lcf;
    lcf = ngx_http_get_module_loc_conf(r, ngx_http_form_input_module);

    ngx_http_form_input_ctx_t *ctx;

    if (lcf->flag == 1) {
        ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);
        if (ctx == NULL) {
            ctx = ngx_palloc(r->pool, sizeof(ngx_http_form_input_ctx_t));
            ngx_http_set_ctx(r, ctx, ngx_http_form_input_module);
            dd("create ngx_http_form_input_ctx");
        }
    }

    return ngx_http_next_header_filter(r);
}
*/
