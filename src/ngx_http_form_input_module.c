#define DDEBUG 1
//#include <ndk.h>
#include "ddebug.h"
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
	ngx_flag_t	flag;
} ngx_http_form_input_loc_conf_t;

typedef struct {
	ngx_int_t done; /*temp var*/
} ngx_http_form_input_ctx_t;

static char *
ngx_http_set_form_input(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void *
ngx_http_form_input_create_loc_conf(ngx_conf_t *cf);

static char *
ngx_http_form_input_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

static ngx_int_t
ngx_http_form_input_init(ngx_conf_t *cf);

static ngx_int_t
ngx_http_form_input_handler(ngx_http_request_t *r);

//static ngx_http_output_header_filter_pt		ngx_http_next_header_filter;

//static ngx_http_output_body_filter_pt		ngx_http_next_body_filter;

//static ngx_int_t ngx_http_form_input_header_filter(ngx_http_request_t *r);

static void ngx_http_form_input_post_read(ngx_http_request_t *r);

static ngx_command_t ngx_http_form_input_commands[] = {

	{ ngx_string("set_form_input"),
	  NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
	  ngx_http_set_form_input,
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

static char *
ngx_http_set_form_input(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	dd("ngx_http_set_form_input");
	ngx_http_form_input_loc_conf_t *filcf;
	filcf = conf;
	filcf->flag = 1;
	dd("form input loc conf:flag=%d", filcf->flag);
	
	return NGX_OK;
}

static void *
ngx_http_form_input_create_loc_conf(ngx_conf_t *cf)
{
	dd("form input create loc conf");
	ngx_http_form_input_loc_conf_t *conf;
	conf = ngx_palloc(cf->pool, sizeof(ngx_http_form_input_loc_conf_t));
	
	if(conf == NULL) {
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

static ngx_int_t
ngx_http_form_input_init(ngx_conf_t *cf)
{
	dd("post conf: form_input_init");
	
	ngx_http_handler_pt			*h;
	ngx_http_core_main_conf_t	*cmcf;

	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

	h = ngx_array_push(&cmcf->phases[NGX_HTTP_REWRITE_PHASE].handlers);
	
	if(h == NULL) {
		return NGX_ERROR;
	}
	
	*h = ngx_http_form_input_handler;
	
//	ngx_http_next_header_filter = ngx_http_top_header_filter;
//	ngx_http_top_header_filter = ngx_http_form_input_header_filter;

	return NGX_OK;

}

static ngx_int_t
ngx_http_form_input_handler(ngx_http_request_t *r)
{
	ngx_http_form_input_loc_conf_t *lcf;
	ngx_http_form_input_ctx_t		*ctx;
	lcf = ngx_http_get_module_loc_conf(r, ngx_http_form_input_module);
/*	
	if(lcf == NULL) {
		return NGX_ERROR;
	}
*/
	if(lcf->flag == 0 || lcf->flag == NGX_CONF_UNSET) {
		dd("handler:declinent");
		return NGX_DECLINED;
	}
	dd("handler:ok");
	
	ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);
	
	if(ctx == NULL) {
		ctx = ngx_palloc(r->pool, sizeof(ngx_http_form_input_ctx_t));
		if(ctx == NULL) {
			return NGX_ERROR;
		}
		ctx->done = 0;
		dd("create module ctx");
	}
	dd("add callback");
	ngx_http_read_client_request_body(r, ngx_http_form_input_post_read);
	
	if(ctx->done == 0) {
		return NGX_OK;
	} else {
		return NGX_OK;
	}
}

static void ngx_http_form_input_post_read(ngx_http_request_t *r)
{
	static int i = 0;
	ngx_http_form_input_ctx_t *ctx;
	dd("start post body read");
	ngx_http_request_body_t *rb;
	
	i++;
	rb = r->request_body;
	if(rb->buf == NULL) {
		dd("rb->buf==NULL");
		return;
	}
	
	/* do process body work here */
	if(i >= 1) {
		ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);
		ctx->done = 1;
	}
	dd("end pb i=%d", i);
}

/*
static ngx_int_t ngx_http_form_input_header_filter(ngx_http_request_t *r)
{
	dd("input header filter");
	ngx_http_form_input_loc_conf_t *lcf;
	lcf = ngx_http_get_module_loc_conf(r, ngx_http_form_input_module);

	ngx_http_form_input_ctx_t *ctx;

	if(lcf->flag == 1) {
		ctx = ngx_http_get_module_ctx(r, ngx_http_form_input_module);
		if(ctx == NULL) {
			ctx = ngx_palloc(r->pool, sizeof(ngx_http_form_input_ctx_t));
			ngx_http_set_ctx(r, ctx, ngx_http_form_input_module);
			dd("create ngx_http_form_input_ctx");
		}
	}

	return ngx_http_next_header_filter(r);
}
*/
